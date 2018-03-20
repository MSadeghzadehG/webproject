

package org.elasticsearch.gateway;

import com.carrotsearch.hppc.cursors.ObjectObjectCursor;
import org.elasticsearch.Version;
import org.elasticsearch.cluster.ClusterChangedEvent;
import org.elasticsearch.cluster.ClusterState;
import org.elasticsearch.cluster.ClusterStateApplier;
import org.elasticsearch.cluster.metadata.IndexMetaData;
import org.elasticsearch.cluster.metadata.MetaData;
import org.elasticsearch.cluster.metadata.MetaDataIndexUpgradeService;
import org.elasticsearch.cluster.node.DiscoveryNode;
import org.elasticsearch.cluster.routing.RoutingNode;
import org.elasticsearch.cluster.routing.ShardRouting;
import org.elasticsearch.common.Nullable;
import org.elasticsearch.common.collect.ImmutableOpenMap;
import org.elasticsearch.common.component.AbstractComponent;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.common.util.IndexFolderUpgrader;
import org.elasticsearch.env.NodeEnvironment;
import org.elasticsearch.index.Index;
import org.elasticsearch.plugins.MetaDataUpgrader;

import java.io.IOException;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.function.BiConsumer;
import java.util.function.Consumer;
import java.util.function.UnaryOperator;

import static java.util.Collections.emptySet;
import static java.util.Collections.unmodifiableSet;

public class GatewayMetaState extends AbstractComponent implements ClusterStateApplier {

    private final NodeEnvironment nodeEnv;
    private final MetaStateService metaStateService;

    @Nullable
    private volatile MetaData previousMetaData;

    private volatile Set<Index> previouslyWrittenIndices = emptySet();

    public GatewayMetaState(Settings settings, NodeEnvironment nodeEnv, MetaStateService metaStateService,
                            MetaDataIndexUpgradeService metaDataIndexUpgradeService, MetaDataUpgrader metaDataUpgrader) throws IOException {
        super(settings);
        this.nodeEnv = nodeEnv;
        this.metaStateService = metaStateService;

        if (DiscoveryNode.isDataNode(settings)) {
            ensureNoPre019ShardState(nodeEnv);
        }

        if (DiscoveryNode.isMasterNode(settings) || DiscoveryNode.isDataNode(settings)) {
            nodeEnv.ensureAtomicMoveSupported();
        }
        if (DiscoveryNode.isMasterNode(settings) || DiscoveryNode.isDataNode(settings)) {
            try {
                ensureNoPre019State();
                IndexFolderUpgrader.upgradeIndicesIfNeeded(settings, nodeEnv);
                final MetaData metaData = metaStateService.loadFullState();
                final MetaData upgradedMetaData = upgradeMetaData(metaData, metaDataIndexUpgradeService, metaDataUpgrader);
                                                                if (metaData != upgradedMetaData) {
                    if (MetaData.isGlobalStateEquals(metaData, upgradedMetaData) == false) {
                        metaStateService.writeGlobalState("upgrade", upgradedMetaData);
                    }
                    for (IndexMetaData indexMetaData : upgradedMetaData) {
                        if (metaData.hasIndexMetaData(indexMetaData) == false) {
                            metaStateService.writeIndex("upgrade", indexMetaData);
                        }
                    }
                }
                long startNS = System.nanoTime();
                metaStateService.loadFullState();
                logger.debug("took {} to load state", TimeValue.timeValueMillis(TimeValue.nsecToMSec(System.nanoTime() - startNS)));
            } catch (Exception e) {
                logger.error("failed to read local state, exiting...", e);
                throw e;
            }
        }
    }

    public MetaData loadMetaState() throws IOException {
        return metaStateService.loadFullState();
    }

    @Override
    public void applyClusterState(ClusterChangedEvent event) {

        final ClusterState state = event.state();
        if (state.blocks().disableStatePersistence()) {
                        this.previousMetaData = null;
            previouslyWrittenIndices = emptySet();
            return;
        }

        MetaData newMetaData = state.metaData();
                Set<Index> relevantIndices = Collections.emptySet();
        boolean success = true;
                if (state.nodes().getLocalNode().isMasterNode() || state.nodes().getLocalNode().isDataNode()) {
            if (previousMetaData == null) {
                try {
                                                                                                                                            if (isDataOnlyNode(state)) {
                        Set<Index> newPreviouslyWrittenIndices = new HashSet<>(previouslyWrittenIndices.size());
                        for (IndexMetaData indexMetaData : newMetaData) {
                            IndexMetaData indexMetaDataOnDisk = null;
                            if (indexMetaData.getState().equals(IndexMetaData.State.CLOSE)) {
                                indexMetaDataOnDisk = metaStateService.loadIndexState(indexMetaData.getIndex());
                            }
                            if (indexMetaDataOnDisk != null) {
                                newPreviouslyWrittenIndices.add(indexMetaDataOnDisk.getIndex());
                            }
                        }
                        newPreviouslyWrittenIndices.addAll(previouslyWrittenIndices);
                        previouslyWrittenIndices = unmodifiableSet(newPreviouslyWrittenIndices);
                    }
                } catch (Exception e) {
                    success = false;
                }
            }
                        if (previousMetaData == null || !MetaData.isGlobalStateEquals(previousMetaData, newMetaData)) {
                try {
                    metaStateService.writeGlobalState("changed", newMetaData);
                } catch (Exception e) {
                    success = false;
                }
            }


            relevantIndices = getRelevantIndices(event.state(), event.previousState(), previouslyWrittenIndices);
            final Iterable<IndexMetaWriteInfo> writeInfo = resolveStatesToBeWritten(previouslyWrittenIndices, relevantIndices, previousMetaData, event.state().metaData());
                        for (IndexMetaWriteInfo indexMetaWrite : writeInfo) {
                try {
                    metaStateService.writeIndex(indexMetaWrite.reason, indexMetaWrite.newMetaData);
                } catch (Exception e) {
                    success = false;
                }
            }
        }

        if (success) {
            previousMetaData = newMetaData;
            previouslyWrittenIndices = unmodifiableSet(relevantIndices);
        }
    }

    public static Set<Index> getRelevantIndices(ClusterState state, ClusterState previousState, Set<Index> previouslyWrittenIndices) {
        Set<Index> relevantIndices;
        if (isDataOnlyNode(state)) {
            relevantIndices = getRelevantIndicesOnDataOnlyNode(state, previousState, previouslyWrittenIndices);
        } else if (state.nodes().getLocalNode().isMasterNode()) {
            relevantIndices = getRelevantIndicesForMasterEligibleNode(state);
        } else {
            relevantIndices = Collections.emptySet();
        }
        return relevantIndices;
    }


    protected static boolean isDataOnlyNode(ClusterState state) {
        return ((state.nodes().getLocalNode().isMasterNode() == false) && state.nodes().getLocalNode().isDataNode());
    }

    
    private void ensureNoPre019State() throws IOException {
        for (Path dataLocation : nodeEnv.nodeDataPaths()) {
            final Path stateLocation = dataLocation.resolve(MetaDataStateFormat.STATE_DIR_NAME);
            if (!Files.exists(stateLocation)) {
                continue;
            }
            try (DirectoryStream<Path> stream = Files.newDirectoryStream(stateLocation)) {
                for (Path stateFile : stream) {
                    if (logger.isTraceEnabled()) {
                        logger.trace("[upgrade]: processing [{}]", stateFile.getFileName());
                    }
                    final String name = stateFile.getFileName().toString();
                    if (name.startsWith("metadata-")) {
                        throw new IllegalStateException("Detected pre 0.19 metadata file please upgrade to a version before "
                            + Version.CURRENT.minimumIndexCompatibilityVersion()
                            + " first to upgrade state structures - metadata found: [" + stateFile.getParent().toAbsolutePath());
                    }
                }
            }
        }
    }

    
    static MetaData upgradeMetaData(MetaData metaData,
                                    MetaDataIndexUpgradeService metaDataIndexUpgradeService,
                                    MetaDataUpgrader metaDataUpgrader) throws IOException {
                boolean changed = false;
        final MetaData.Builder upgradedMetaData = MetaData.builder(metaData);
        for (IndexMetaData indexMetaData : metaData) {
            IndexMetaData newMetaData = metaDataIndexUpgradeService.upgradeIndexMetaData(indexMetaData,
                Version.CURRENT.minimumIndexCompatibilityVersion());
            changed |= indexMetaData != newMetaData;
            upgradedMetaData.put(newMetaData, false);
        }
                if (applyPluginUpgraders(metaData.getCustoms(), metaDataUpgrader.customMetaDataUpgraders,
            upgradedMetaData::removeCustom,upgradedMetaData::putCustom)) {
            changed = true;
        }
                if (applyPluginUpgraders(metaData.getTemplates(), metaDataUpgrader.indexTemplateMetaDataUpgraders,
            upgradedMetaData::removeTemplate, (s, indexTemplateMetaData) -> upgradedMetaData.put(indexTemplateMetaData))) {
            changed = true;
        }
        return changed ? upgradedMetaData.build() : metaData;
    }

    private static <Data> boolean applyPluginUpgraders(ImmutableOpenMap<String, Data> existingData,
                                                       UnaryOperator<Map<String, Data>> upgrader,
                                                       Consumer<String> removeData,
                                                       BiConsumer<String, Data> putData) {
                Map<String, Data> existingMap = new HashMap<>();
        for (ObjectObjectCursor<String, Data> customCursor : existingData) {
            existingMap.put(customCursor.key, customCursor.value);
        }
                Map<String, Data> upgradedCustoms = upgrader.apply(existingMap);
        if (upgradedCustoms.equals(existingMap) == false) {
                        existingMap.keySet().forEach(removeData);
            for (Map.Entry<String, Data> upgradedCustomEntry : upgradedCustoms.entrySet()) {
                putData.accept(upgradedCustomEntry.getKey(), upgradedCustomEntry.getValue());
            }
            return true;
        }
        return false;
    }

        private void ensureNoPre019ShardState(NodeEnvironment nodeEnv) throws IOException {
        for (Path dataLocation : nodeEnv.nodeDataPaths()) {
            final Path stateLocation = dataLocation.resolve(MetaDataStateFormat.STATE_DIR_NAME);
            if (Files.exists(stateLocation)) {
                try (DirectoryStream<Path> stream = Files.newDirectoryStream(stateLocation, "shards-*")) {
                    for (Path stateFile : stream) {
                        throw new IllegalStateException("Detected pre 0.19 shard state file please upgrade to a version before "
                                + Version.CURRENT.minimumIndexCompatibilityVersion()
                                + " first to upgrade state structures - shard state found: [" + stateFile.getParent().toAbsolutePath());
                    }
                }
            }
        }
    }

    
    public static Iterable<GatewayMetaState.IndexMetaWriteInfo> resolveStatesToBeWritten(Set<Index> previouslyWrittenIndices, Set<Index> potentiallyUnwrittenIndices, MetaData previousMetaData, MetaData newMetaData) {
        List<GatewayMetaState.IndexMetaWriteInfo> indicesToWrite = new ArrayList<>();
        for (Index index : potentiallyUnwrittenIndices) {
            IndexMetaData newIndexMetaData = newMetaData.getIndexSafe(index);
            IndexMetaData previousIndexMetaData = previousMetaData == null ? null : previousMetaData.index(index);
            String writeReason = null;
            if (previouslyWrittenIndices.contains(index) == false || previousIndexMetaData == null) {
                writeReason = "freshly created";
            } else if (previousIndexMetaData.getVersion() != newIndexMetaData.getVersion()) {
                writeReason = "version changed from [" + previousIndexMetaData.getVersion() + "] to [" + newIndexMetaData.getVersion() + "]";
            }
            if (writeReason != null) {
                indicesToWrite.add(new GatewayMetaState.IndexMetaWriteInfo(newIndexMetaData, previousIndexMetaData, writeReason));
            }
        }
        return indicesToWrite;
    }

    public static Set<Index> getRelevantIndicesOnDataOnlyNode(ClusterState state, ClusterState previousState, Set<Index> previouslyWrittenIndices) {
        RoutingNode newRoutingNode = state.getRoutingNodes().node(state.nodes().getLocalNodeId());
        if (newRoutingNode == null) {
            throw new IllegalStateException("cluster state does not contain this node - cannot write index meta state");
        }
        Set<Index> indices = new HashSet<>();
        for (ShardRouting routing : newRoutingNode) {
            indices.add(routing.index());
        }
                for (IndexMetaData indexMetaData : state.metaData()) {
            boolean isOrWasClosed = indexMetaData.getState().equals(IndexMetaData.State.CLOSE);
                                    IndexMetaData previousMetaData = previousState.metaData().index(indexMetaData.getIndex());
            if (previousMetaData != null) {
                isOrWasClosed = isOrWasClosed || previousMetaData.getState().equals(IndexMetaData.State.CLOSE);
            }
            if (previouslyWrittenIndices.contains(indexMetaData.getIndex()) && isOrWasClosed) {
                indices.add(indexMetaData.getIndex());
            }
        }
        return indices;
    }

    public static Set<Index> getRelevantIndicesForMasterEligibleNode(ClusterState state) {
        Set<Index> relevantIndices;
        relevantIndices = new HashSet<>();
                for (IndexMetaData indexMetaData : state.metaData()) {
            relevantIndices.add(indexMetaData.getIndex());
        }
        return relevantIndices;
    }


    public static class IndexMetaWriteInfo {
        final IndexMetaData newMetaData;
        final String reason;
        final IndexMetaData previousMetaData;

        public IndexMetaWriteInfo(IndexMetaData newMetaData, IndexMetaData previousMetaData, String reason) {
            this.newMetaData = newMetaData;
            this.reason = reason;
            this.previousMetaData = previousMetaData;
        }

        public IndexMetaData getNewMetaData() {
            return newMetaData;
        }

        public String getReason() {
            return reason;
        }
    }
}
