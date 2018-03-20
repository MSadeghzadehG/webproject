

package org.elasticsearch.discovery.gce;

import java.io.IOException;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.function.Function;

import com.google.api.services.compute.model.AccessConfig;
import com.google.api.services.compute.model.Instance;
import com.google.api.services.compute.model.NetworkInterface;
import org.apache.logging.log4j.message.ParameterizedMessage;
import org.apache.logging.log4j.util.Supplier;
import org.elasticsearch.Version;
import org.elasticsearch.cloud.gce.GceInstancesService;
import org.elasticsearch.cluster.node.DiscoveryNode;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.component.AbstractComponent;
import org.elasticsearch.common.network.NetworkAddress;
import org.elasticsearch.common.network.NetworkService;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Setting.Property;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.transport.TransportAddress;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.discovery.zen.UnicastHostsProvider;
import org.elasticsearch.transport.TransportService;

import static java.util.Collections.emptyList;
import static java.util.Collections.emptyMap;
import static java.util.Collections.emptySet;

public class GceUnicastHostsProvider extends AbstractComponent implements UnicastHostsProvider {

    
    public static final Setting<List<String>> TAGS_SETTING =
            Setting.listSetting("discovery.gce.tags", emptyList(), Function.identity(), Property.NodeScope);

    static final class Status {
        private static final String TERMINATED = "TERMINATED";
    }

    private final GceInstancesService gceInstancesService;
    private TransportService transportService;
    private NetworkService networkService;

    private final String project;
    private final List<String> zones;
    private final List<String> tags;

    private final TimeValue refreshInterval;
    private long lastRefresh;
    private List<DiscoveryNode> cachedDiscoNodes;

    public GceUnicastHostsProvider(Settings settings, GceInstancesService gceInstancesService,
            TransportService transportService,
            NetworkService networkService) {
        super(settings);
        this.gceInstancesService = gceInstancesService;
        this.transportService = transportService;
        this.networkService = networkService;

        this.refreshInterval = GceInstancesService.REFRESH_SETTING.get(settings);
        this.project = GceInstancesService.PROJECT_SETTING.get(settings);
        this.zones = GceInstancesService.ZONE_SETTING.get(settings);

        this.tags = TAGS_SETTING.get(settings);
        if (logger.isDebugEnabled()) {
            logger.debug("using tags {}", this.tags);
        }
    }

    
    @Override
    public List<DiscoveryNode> buildDynamicNodes() {
                if (this.project == null || this.project.isEmpty() || this.zones == null || this.zones.isEmpty()) {
            throw new IllegalArgumentException("one or more gce discovery settings are missing. " +
                "Check elasticsearch.yml file. Should have [" + GceInstancesService.PROJECT_SETTING.getKey() +
                "] and [" + GceInstancesService.ZONE_SETTING.getKey() + "].");
        }

        if (refreshInterval.millis() != 0) {
            if (cachedDiscoNodes != null &&
                    (refreshInterval.millis() < 0 || (System.currentTimeMillis() - lastRefresh) < refreshInterval.millis())) {
                if (logger.isTraceEnabled()) logger.trace("using cache to retrieve node list");
                return cachedDiscoNodes;
            }
            lastRefresh = System.currentTimeMillis();
        }
        logger.debug("start building nodes list using GCE API");

        cachedDiscoNodes = new ArrayList<>();
        String ipAddress = null;
        try {
            InetAddress inetAddress = networkService.resolvePublishHostAddresses(
                NetworkService.GLOBAL_NETWORK_PUBLISHHOST_SETTING.get(settings).toArray(Strings.EMPTY_ARRAY));
            if (inetAddress != null) {
                ipAddress = NetworkAddress.format(inetAddress);
            }
        } catch (IOException e) {
                                }

        try {
            Collection<Instance> instances = gceInstancesService.instances();

            if (instances == null) {
                logger.trace("no instance found for project [{}], zones [{}].", this.project, this.zones);
                return cachedDiscoNodes;
            }

            for (Instance instance : instances) {
                String name = instance.getName();
                String type = instance.getMachineType();

                String status = instance.getStatus();
                logger.trace("gce instance {} with status {} found.", name, status);

                                                if (Status.TERMINATED.equals(status)) {
                    logger.debug("node {} is TERMINATED. Ignoring", name);
                    continue;
                }

                                boolean filterByTag = false;
                if (tags.isEmpty() == false) {
                    logger.trace("start filtering instance {} with tags {}.", name, tags);
                    if (instance.getTags() == null || instance.getTags().isEmpty()
                            || instance.getTags().getItems() == null || instance.getTags().getItems().isEmpty()) {
                                                logger.trace("no tags for this instance but we asked for tags. {} won't be part of the cluster.", name);
                        filterByTag = true;
                    } else {
                                                logger.trace("comparing instance tags {} with tags filter {}.", instance.getTags().getItems(), tags);
                        for (String tag : tags) {
                            boolean found = false;
                            for (String instancetag : instance.getTags().getItems()) {
                                if (instancetag.equals(tag)) {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                filterByTag = true;
                                break;
                            }
                        }
                    }
                }
                if (filterByTag) {
                    logger.trace("filtering out instance {} based tags {}, not part of {}", name, tags,
                            instance.getTags() == null || instance.getTags().getItems() == null ? "" : instance.getTags());
                    continue;
                } else {
                    logger.trace("instance {} with tags {} is added to discovery", name, tags);
                }

                String ip_public = null;
                String ip_private = null;

                List<NetworkInterface> interfaces = instance.getNetworkInterfaces();

                for (NetworkInterface networkInterface : interfaces) {
                    if (ip_public == null) {
                                                if (networkInterface.getAccessConfigs() != null) {
                            for (AccessConfig accessConfig : networkInterface.getAccessConfigs()) {
                                if (Strings.hasText(accessConfig.getNatIP())) {
                                    ip_public = accessConfig.getNatIP();
                                    break;
                                }
                            }
                        }
                    }

                    if (ip_private == null) {
                        ip_private = networkInterface.getNetworkIP();
                    }

                                        if (ip_private != null && ip_public != null) break;
                }

                try {
                    if (ip_private.equals(ipAddress)) {
                                                                        logger.trace("current node found. Ignoring {} - {}", name, ip_private);
                    } else {
                        String address = ip_private;
                                                if (instance.getMetadata() != null && instance.getMetadata().containsKey("es_port")) {
                            Object es_port = instance.getMetadata().get("es_port");
                            logger.trace("es_port is defined with {}", es_port);
                            if (es_port instanceof String) {
                                address = address.concat(":").concat((String) es_port);
                            } else {
                                                                logger.trace("es_port is instance of {}. Ignoring...", es_port.getClass().getName());
                            }
                        }

                                                                                                TransportAddress[] addresses = transportService.addressesFromString(address, 1);

                        for (TransportAddress transportAddress : addresses) {
                            logger.trace("adding {}, type {}, address {}, transport_address {}, status {}", name, type,
                                    ip_private, transportAddress, status);
                            cachedDiscoNodes.add(new DiscoveryNode("#cloud-" + name + "-" + 0, transportAddress,
                                    emptyMap(), emptySet(), Version.CURRENT.minimumCompatibilityVersion()));
                        }
                    }
                } catch (Exception e) {
                    final String finalIpPrivate = ip_private;
                    logger.warn((Supplier<?>) () -> new ParameterizedMessage("failed to add {}, address {}", name, finalIpPrivate), e);
                }

            }
        } catch (Exception e) {
            logger.warn("exception caught during discovery", e);
        }

        logger.debug("{} node(s) added", cachedDiscoNodes.size());
        logger.debug("using dynamic discovery nodes {}", cachedDiscoNodes);

        return cachedDiscoNodes;
    }
}
