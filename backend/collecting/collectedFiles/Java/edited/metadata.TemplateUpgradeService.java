

package org.elasticsearch.cluster.metadata;

import com.carrotsearch.hppc.cursors.ObjectCursor;
import com.carrotsearch.hppc.cursors.ObjectObjectCursor;
import org.apache.logging.log4j.message.ParameterizedMessage;
import org.elasticsearch.Version;
import org.elasticsearch.action.ActionListener;
import org.elasticsearch.action.admin.indices.template.delete.DeleteIndexTemplateRequest;
import org.elasticsearch.action.admin.indices.template.delete.DeleteIndexTemplateResponse;
import org.elasticsearch.action.admin.indices.template.put.PutIndexTemplateRequest;
import org.elasticsearch.action.admin.indices.template.put.PutIndexTemplateResponse;
import org.elasticsearch.client.Client;
import org.elasticsearch.cluster.ClusterChangedEvent;
import org.elasticsearch.cluster.ClusterState;
import org.elasticsearch.cluster.ClusterStateListener;
import org.elasticsearch.cluster.node.DiscoveryNode;
import org.elasticsearch.cluster.node.DiscoveryNodes;
import org.elasticsearch.cluster.service.ClusterService;
import org.elasticsearch.common.bytes.BytesReference;
import org.elasticsearch.common.collect.ImmutableOpenMap;
import org.elasticsearch.common.collect.Tuple;
import org.elasticsearch.common.component.AbstractComponent;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.common.xcontent.ToXContent;
import org.elasticsearch.common.xcontent.XContentHelper;
import org.elasticsearch.common.xcontent.XContentType;
import org.elasticsearch.gateway.GatewayService;
import org.elasticsearch.indices.IndexTemplateMissingException;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.threadpool.ThreadPool;

import java.io.IOException;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.UnaryOperator;

import static java.util.Collections.singletonMap;


public class TemplateUpgradeService extends AbstractComponent implements ClusterStateListener {
    private final UnaryOperator<Map<String, IndexTemplateMetaData>> indexTemplateMetaDataUpgraders;

    public final ClusterService clusterService;

    public final ThreadPool threadPool;

    public final Client client;

    private final AtomicInteger updatesInProgress = new AtomicInteger();

    private ImmutableOpenMap<String, IndexTemplateMetaData> lastTemplateMetaData;

    public TemplateUpgradeService(Settings settings, Client client, ClusterService clusterService, ThreadPool threadPool,
                                  Collection<UnaryOperator<Map<String, IndexTemplateMetaData>>> indexTemplateMetaDataUpgraders) {
        super(settings);
        this.client = client;
        this.clusterService = clusterService;
        this.threadPool = threadPool;
        this.indexTemplateMetaDataUpgraders = templates -> {
            Map<String, IndexTemplateMetaData> upgradedTemplates = new HashMap<>(templates);
            for (UnaryOperator<Map<String, IndexTemplateMetaData>> upgrader : indexTemplateMetaDataUpgraders) {
                upgradedTemplates = upgrader.apply(upgradedTemplates);
            }
            return upgradedTemplates;
        };
        clusterService.addListener(this);
    }

    @Override
    public void clusterChanged(ClusterChangedEvent event) {
        ClusterState state = event.state();
        if (state.blocks().hasGlobalBlock(GatewayService.STATE_NOT_RECOVERED_BLOCK)) {
                                    return;
        }

        if (updatesInProgress.get() > 0) {
                        return;
        }

        ImmutableOpenMap<String, IndexTemplateMetaData> templates = state.getMetaData().getTemplates();

        if (templates == lastTemplateMetaData) {
                                                return;
        }

        if (state.nodes().isLocalNodeElectedMaster() == false) {
            return;
        }

        lastTemplateMetaData = templates;
        Optional<Tuple<Map<String, BytesReference>, Set<String>>> changes = calculateTemplateChanges(templates);
        if (changes.isPresent()) {
            if (updatesInProgress.compareAndSet(0, changes.get().v1().size() + changes.get().v2().size())) {
                logger.info("Starting template upgrade to version {}, {} templates will be updated and {} will be removed",
                    Version.CURRENT,
                    changes.get().v1().size(),
                    changes.get().v2().size());
                threadPool.generic().execute(() -> updateTemplates(changes.get().v1(), changes.get().v2()));
            }
        }
    }

    void updateTemplates(Map<String, BytesReference> changes, Set<String> deletions) {
        for (Map.Entry<String, BytesReference> change : changes.entrySet()) {
            PutIndexTemplateRequest request =
                new PutIndexTemplateRequest(change.getKey()).source(change.getValue(), XContentType.JSON);
            request.masterNodeTimeout(TimeValue.timeValueMinutes(1));
            client.admin().indices().putTemplate(request, new ActionListener<PutIndexTemplateResponse>() {
                @Override
                public void onResponse(PutIndexTemplateResponse response) {
                    if(updatesInProgress.decrementAndGet() == 0) {
                        logger.info("Finished upgrading templates to version {}", Version.CURRENT);
                    }
                    if (response.isAcknowledged() == false) {
                        logger.warn("Error updating template [{}], request was not acknowledged", change.getKey());
                    }
                }

                @Override
                public void onFailure(Exception e) {
                    if(updatesInProgress.decrementAndGet() == 0) {
                        logger.info("Templates were upgraded to version {}", Version.CURRENT);
                    }
                    logger.warn(new ParameterizedMessage("Error updating template [{}]", change.getKey()), e);
                }
            });
        }

        for (String template : deletions) {
            DeleteIndexTemplateRequest request = new DeleteIndexTemplateRequest(template);
            request.masterNodeTimeout(TimeValue.timeValueMinutes(1));
            client.admin().indices().deleteTemplate(request, new ActionListener<DeleteIndexTemplateResponse>() {
                @Override
                public void onResponse(DeleteIndexTemplateResponse response) {
                    updatesInProgress.decrementAndGet();
                    if (response.isAcknowledged() == false) {
                        logger.warn("Error deleting template [{}], request was not acknowledged", template);
                    }
                }

                @Override
                public void onFailure(Exception e) {
                    updatesInProgress.decrementAndGet();
                    if (e instanceof IndexTemplateMissingException == false) {
                                                                        logger.warn(new ParameterizedMessage("Error deleting template [{}]", template), e);
                    }
                }
            });
        }
    }

    int getUpdatesInProgress() {
        return updatesInProgress.get();
    }

    Optional<Tuple<Map<String, BytesReference>, Set<String>>> calculateTemplateChanges(
        ImmutableOpenMap<String, IndexTemplateMetaData> templates) {
                Map<String, IndexTemplateMetaData> existingMap = new HashMap<>();
        for (ObjectObjectCursor<String, IndexTemplateMetaData> customCursor : templates) {
            existingMap.put(customCursor.key, customCursor.value);
        }
                Map<String, IndexTemplateMetaData> upgradedMap = indexTemplateMetaDataUpgraders.apply(existingMap);
        if (upgradedMap.equals(existingMap) == false) {
            Set<String> deletes = new HashSet<>();
            Map<String, BytesReference> changes = new HashMap<>();
                        existingMap.keySet().forEach(s -> {
                if (upgradedMap.containsKey(s) == false) {
                    deletes.add(s);
                }
            });
            upgradedMap.forEach((key, value) -> {
                if (value.equals(existingMap.get(key)) == false) {
                    changes.put(key, toBytesReference(value));
                }
            });
            return Optional.of(new Tuple<>(changes, deletes));
        }
        return Optional.empty();
    }

    private static final ToXContent.Params PARAMS = new ToXContent.MapParams(singletonMap("reduce_mappings", "true"));

    private BytesReference toBytesReference(IndexTemplateMetaData templateMetaData) {
        try {
            return XContentHelper.toXContent((builder, params) -> {
                IndexTemplateMetaData.Builder.toInnerXContent(templateMetaData, builder, params);
                return builder;
            }, XContentType.JSON, PARAMS, false);
        } catch (IOException ex) {
            throw new IllegalStateException("Cannot serialize template [" + templateMetaData.getName() + "]", ex);
        }
    }
}
