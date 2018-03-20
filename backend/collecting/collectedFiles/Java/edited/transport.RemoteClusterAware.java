
package org.elasticsearch.transport;

import org.elasticsearch.Version;
import org.elasticsearch.cluster.metadata.ClusterNameExpressionResolver;
import org.elasticsearch.cluster.node.DiscoveryNode;
import org.elasticsearch.common.component.AbstractComponent;
import org.elasticsearch.common.settings.ClusterSettings;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.transport.TransportAddress;

import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.function.Predicate;
import java.util.stream.Collectors;
import java.util.stream.Stream;


public abstract class RemoteClusterAware extends AbstractComponent {

    
    public static final Setting.AffixSetting<List<InetSocketAddress>> REMOTE_CLUSTERS_SEEDS = Setting.affixKeySetting("search.remote.",
        "seeds", (key) -> Setting.listSetting(key, Collections.emptyList(), RemoteClusterAware::parseSeedAddress,
            Setting.Property.NodeScope, Setting.Property.Dynamic));
    public static final char REMOTE_CLUSTER_INDEX_SEPARATOR = ':';
    public static final String LOCAL_CLUSTER_GROUP_KEY = "";

    protected final ClusterNameExpressionResolver clusterNameResolver;

    
    protected RemoteClusterAware(Settings settings) {
        super(settings);
        this.clusterNameResolver = new ClusterNameExpressionResolver(settings);
    }

    protected static Map<String, List<DiscoveryNode>> buildRemoteClustersSeeds(Settings settings) {
        Stream<Setting<List<InetSocketAddress>>> allConcreteSettings = REMOTE_CLUSTERS_SEEDS.getAllConcreteSettings(settings);
        return allConcreteSettings.collect(
            Collectors.toMap(REMOTE_CLUSTERS_SEEDS::getNamespace, concreteSetting -> {
                String clusterName = REMOTE_CLUSTERS_SEEDS.getNamespace(concreteSetting);
                List<DiscoveryNode> nodes = new ArrayList<>();
                for (InetSocketAddress address : concreteSetting.get(settings)) {
                    TransportAddress transportAddress = new TransportAddress(address);
                    DiscoveryNode node = new DiscoveryNode(clusterName + "#" + transportAddress.toString(),
                        transportAddress,
                        Version.CURRENT.minimumCompatibilityVersion());
                    nodes.add(node);
                }
                return nodes;
            }));
    }

    
    public Map<String, List<String>> groupClusterIndices(String[] requestIndices, Predicate<String> indexExists) {
        Map<String, List<String>> perClusterIndices = new HashMap<>();
        Set<String> remoteClusterNames = getRemoteClusterNames();
        for (String index : requestIndices) {
            int i = index.indexOf(RemoteClusterService.REMOTE_CLUSTER_INDEX_SEPARATOR);
            if (i >= 0) {
                String remoteClusterName = index.substring(0, i);
                List<String> clusters = clusterNameResolver.resolveClusterNames(remoteClusterNames, remoteClusterName);
                if (clusters.isEmpty() == false) {
                    if (indexExists.test(index)) {
                                                                                                throw new IllegalArgumentException("Can not filter indices; index " + index +
                            " exists but there is also a remote cluster named: " + remoteClusterName);
                        }
                    String indexName = index.substring(i + 1);
                    for (String clusterName : clusters) {
                        perClusterIndices.computeIfAbsent(clusterName, k -> new ArrayList<>()).add(indexName);
                    }
                } else {
                    perClusterIndices.computeIfAbsent(RemoteClusterAware.LOCAL_CLUSTER_GROUP_KEY, k -> new ArrayList<>()).add(index);
                }
            } else {
                perClusterIndices.computeIfAbsent(RemoteClusterAware.LOCAL_CLUSTER_GROUP_KEY, k -> new ArrayList<>()).add(index);
            }
        }
        return perClusterIndices;
    }

    protected abstract Set<String> getRemoteClusterNames();

    
    protected abstract void updateRemoteCluster(String clusterAlias, List<InetSocketAddress> addresses);

    
    public void listenForUpdates(ClusterSettings clusterSettings) {
        clusterSettings.addAffixUpdateConsumer(RemoteClusterAware.REMOTE_CLUSTERS_SEEDS, this::updateRemoteCluster,
            (namespace, value) -> {});
    }

    private static InetSocketAddress parseSeedAddress(String remoteHost) {
        int portSeparator = remoteHost.lastIndexOf(':');         if (portSeparator == -1 || portSeparator == remoteHost.length()) {
            throw new IllegalArgumentException("remote hosts need to be configured as [host:port], found [" + remoteHost + "] instead");
        }
        String host = remoteHost.substring(0, portSeparator);
        InetAddress hostAddress;
        try {
            hostAddress = InetAddress.getByName(host);
        } catch (UnknownHostException e) {
            throw new IllegalArgumentException("unknown host [" + host + "]", e);
        }
        try {
            int port = Integer.valueOf(remoteHost.substring(portSeparator + 1));
            if (port <= 0) {
                throw new IllegalArgumentException("port number must be > 0 but was: [" + port + "]");
            }
            return new InetSocketAddress(hostAddress, port);
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("port must be a number", e);
        }
    }

    public static String buildRemoteIndexName(String clusterAlias, String indexName) {
        return clusterAlias != null ? clusterAlias + REMOTE_CLUSTER_INDEX_SEPARATOR + indexName : indexName;
    }
}
