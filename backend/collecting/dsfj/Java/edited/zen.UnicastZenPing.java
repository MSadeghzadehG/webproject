

package org.elasticsearch.discovery.zen;

import com.carrotsearch.hppc.cursors.ObjectCursor;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.message.ParameterizedMessage;
import org.apache.logging.log4j.util.Supplier;
import org.apache.lucene.store.AlreadyClosedException;
import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.Version;
import org.elasticsearch.cluster.ClusterName;
import org.elasticsearch.cluster.ClusterState;
import org.elasticsearch.cluster.node.DiscoveryNode;
import org.elasticsearch.cluster.node.DiscoveryNodes;
import org.elasticsearch.common.component.AbstractComponent;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.lease.Releasable;
import org.elasticsearch.common.lease.Releasables;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Setting.Property;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.transport.TransportAddress;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.common.util.CollectionUtils;
import org.elasticsearch.common.util.concurrent.AbstractRunnable;
import org.elasticsearch.common.util.concurrent.ConcurrentCollections;
import org.elasticsearch.common.util.concurrent.EsExecutors;
import org.elasticsearch.common.util.concurrent.EsThreadPoolExecutor;
import org.elasticsearch.common.util.concurrent.KeyedLock;
import org.elasticsearch.threadpool.ThreadPool;
import org.elasticsearch.transport.ConnectTransportException;
import org.elasticsearch.transport.ConnectionProfile;
import org.elasticsearch.transport.NodeNotConnectedException;
import org.elasticsearch.transport.RemoteTransportException;
import org.elasticsearch.transport.Transport.Connection;
import org.elasticsearch.transport.TransportChannel;
import org.elasticsearch.transport.TransportException;
import org.elasticsearch.transport.TransportRequest;
import org.elasticsearch.transport.TransportRequestHandler;
import org.elasticsearch.transport.TransportRequestOptions;
import org.elasticsearch.transport.TransportResponse;
import org.elasticsearch.transport.TransportResponseHandler;
import org.elasticsearch.transport.TransportService;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Queue;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import static java.util.Collections.emptyList;
import static java.util.Collections.emptyMap;
import static java.util.Collections.emptySet;
import static org.elasticsearch.common.util.concurrent.ConcurrentCollections.newConcurrentMap;

public class UnicastZenPing extends AbstractComponent implements ZenPing {

    public static final String ACTION_NAME = "internal:discovery/zen/unicast";
    public static final Setting<List<String>> DISCOVERY_ZEN_PING_UNICAST_HOSTS_SETTING =
        Setting.listSetting("discovery.zen.ping.unicast.hosts", emptyList(), Function.identity(),
            Property.NodeScope);
    public static final Setting<Integer> DISCOVERY_ZEN_PING_UNICAST_CONCURRENT_CONNECTS_SETTING =
        Setting.intSetting("discovery.zen.ping.unicast.concurrent_connects", 10, 0, Property.NodeScope);
    public static final Setting<TimeValue> DISCOVERY_ZEN_PING_UNICAST_HOSTS_RESOLVE_TIMEOUT =
        Setting.positiveTimeSetting("discovery.zen.ping.unicast.hosts.resolve_timeout", TimeValue.timeValueSeconds(5), Property.NodeScope);

        public static final int LIMIT_FOREIGN_PORTS_COUNT = 1;
    public static final int LIMIT_LOCAL_PORTS_COUNT = 5;

    private final ThreadPool threadPool;
    private final TransportService transportService;
    private final ClusterName clusterName;

    private final List<String> configuredHosts;

    private final int limitPortCounts;

    private final PingContextProvider contextProvider;

    private final AtomicInteger pingingRoundIdGenerator = new AtomicInteger();

        private static final String UNICAST_NODE_PREFIX = "#zen_unicast_";

    private final Map<Integer, PingingRound> activePingingRounds = newConcurrentMap();

        private final Queue<PingResponse> temporalResponses = ConcurrentCollections.newQueue();

    private final UnicastHostsProvider hostsProvider;

    protected final EsThreadPoolExecutor unicastZenPingExecutorService;

    private final TimeValue resolveTimeout;

    private volatile boolean closed = false;

    public UnicastZenPing(Settings settings, ThreadPool threadPool, TransportService transportService,
                          UnicastHostsProvider unicastHostsProvider, PingContextProvider contextProvider) {
        super(settings);
        this.threadPool = threadPool;
        this.transportService = transportService;
        this.clusterName = ClusterName.CLUSTER_NAME_SETTING.get(settings);
        this.hostsProvider = unicastHostsProvider;
        this.contextProvider = contextProvider;

        final int concurrentConnects = DISCOVERY_ZEN_PING_UNICAST_CONCURRENT_CONNECTS_SETTING.get(settings);
        if (DISCOVERY_ZEN_PING_UNICAST_HOSTS_SETTING.exists(settings)) {
            configuredHosts = DISCOVERY_ZEN_PING_UNICAST_HOSTS_SETTING.get(settings);
                        limitPortCounts = LIMIT_FOREIGN_PORTS_COUNT;
        } else {
                        configuredHosts = transportService.getLocalAddresses();
            limitPortCounts = LIMIT_LOCAL_PORTS_COUNT;
        }
        resolveTimeout = DISCOVERY_ZEN_PING_UNICAST_HOSTS_RESOLVE_TIMEOUT.get(settings);
        logger.debug(
            "using initial hosts {}, with concurrent_connects [{}], resolve_timeout [{}]",
            configuredHosts,
            concurrentConnects,
            resolveTimeout);

        transportService.registerRequestHandler(ACTION_NAME, ThreadPool.Names.SAME, UnicastPingRequest::new,
            new UnicastPingRequestHandler());

        final ThreadFactory threadFactory = EsExecutors.daemonThreadFactory(settings, "[unicast_connect]");
        unicastZenPingExecutorService = EsExecutors.newScaling(
                nodeName() + "/" + "unicast_connect",
                0,
                concurrentConnects,
                60,
                TimeUnit.SECONDS,
                threadFactory,
                threadPool.getThreadContext());
    }

    
    public static List<DiscoveryNode> resolveHostsLists(
        final ExecutorService executorService,
        final Logger logger,
        final List<String> hosts,
        final int limitPortCounts,
        final TransportService transportService,
        final String nodeId_prefix,
        final TimeValue resolveTimeout) throws InterruptedException {
        Objects.requireNonNull(executorService);
        Objects.requireNonNull(logger);
        Objects.requireNonNull(hosts);
        Objects.requireNonNull(transportService);
        Objects.requireNonNull(nodeId_prefix);
        Objects.requireNonNull(resolveTimeout);
        if (resolveTimeout.nanos() < 0) {
            throw new IllegalArgumentException("resolve timeout must be non-negative but was [" + resolveTimeout + "]");
        }
                final List<Callable<TransportAddress[]>> callables =
            hosts
                .stream()
                .map(hn -> (Callable<TransportAddress[]>) () -> transportService.addressesFromString(hn, limitPortCounts))
                .collect(Collectors.toList());
        final List<Future<TransportAddress[]>> futures =
            executorService.invokeAll(callables, resolveTimeout.nanos(), TimeUnit.NANOSECONDS);
        final List<DiscoveryNode> discoveryNodes = new ArrayList<>();
        final Set<TransportAddress> localAddresses = new HashSet<>();
        localAddresses.add(transportService.boundAddress().publishAddress());
        localAddresses.addAll(Arrays.asList(transportService.boundAddress().boundAddresses()));
                        final Iterator<String> it = hosts.iterator();
        for (final Future<TransportAddress[]> future : futures) {
            final String hostname = it.next();
            if (!future.isCancelled()) {
                assert future.isDone();
                try {
                    final TransportAddress[] addresses = future.get();
                    logger.trace("resolved host [{}] to {}", hostname, addresses);
                    for (int addressId = 0; addressId < addresses.length; addressId++) {
                        final TransportAddress address = addresses[addressId];
                                                if (localAddresses.contains(address) == false) {
                            discoveryNodes.add(
                                new DiscoveryNode(
                                    nodeId_prefix + hostname + "_" + addressId + "#",
                                    address,
                                    emptyMap(),
                                    emptySet(),
                                    Version.CURRENT.minimumCompatibilityVersion()));
                        }
                    }
                } catch (final ExecutionException e) {
                    assert e.getCause() != null;
                    final String message = "failed to resolve host [" + hostname + "]";
                    logger.warn(message, e.getCause());
                }
            } else {
                logger.warn("timed out after [{}] resolving host [{}]", resolveTimeout, hostname);
            }
        }
        return discoveryNodes;
    }

    @Override
    public void close() {
        ThreadPool.terminate(unicastZenPingExecutorService, 10, TimeUnit.SECONDS);
        Releasables.close(activePingingRounds.values());
        closed = true;
    }

    @Override
    public void start() {
    }

    
    public void clearTemporalResponses() {
        temporalResponses.clear();
    }

    
    @Override
    public void ping(final Consumer<PingCollection> resultsConsumer, final TimeValue duration) {
        ping(resultsConsumer, duration, duration);
    }

    
    protected void ping(final Consumer<PingCollection> resultsConsumer,
                        final TimeValue scheduleDuration,
                        final TimeValue requestDuration) {
        final List<DiscoveryNode> seedNodes;
        try {
            seedNodes = resolveHostsLists(
                unicastZenPingExecutorService,
                logger,
                configuredHosts,
                limitPortCounts,
                transportService,
                UNICAST_NODE_PREFIX,
                resolveTimeout);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
        seedNodes.addAll(hostsProvider.buildDynamicNodes());
        final DiscoveryNodes nodes = contextProvider.clusterState().nodes();
                for (ObjectCursor<DiscoveryNode> masterNode : nodes.getMasterNodes().values()) {
            seedNodes.add(masterNode.value);
        }

        final ConnectionProfile connectionProfile =
            ConnectionProfile.buildSingleChannelProfile(TransportRequestOptions.Type.REG, requestDuration, requestDuration);
        final PingingRound pingingRound = new PingingRound(pingingRoundIdGenerator.incrementAndGet(), seedNodes, resultsConsumer,
            nodes.getLocalNode(), connectionProfile);
        activePingingRounds.put(pingingRound.id(), pingingRound);
        final AbstractRunnable pingSender = new AbstractRunnable() {
            @Override
            public void onFailure(Exception e) {
                if (e instanceof AlreadyClosedException == false) {
                    logger.warn("unexpected error while pinging", e);
                }
            }

            @Override
            protected void doRun() throws Exception {
                sendPings(requestDuration, pingingRound);
            }
        };
        threadPool.generic().execute(pingSender);
        threadPool.schedule(TimeValue.timeValueMillis(scheduleDuration.millis() / 3), ThreadPool.Names.GENERIC, pingSender);
        threadPool.schedule(TimeValue.timeValueMillis(scheduleDuration.millis() / 3 * 2), ThreadPool.Names.GENERIC, pingSender);
        threadPool.schedule(scheduleDuration, ThreadPool.Names.GENERIC, new AbstractRunnable() {
            @Override
            protected void doRun() throws Exception {
                finishPingingRound(pingingRound);
            }

            @Override
            public void onFailure(Exception e) {
                logger.warn("unexpected error while finishing pinging round", e);
            }
        });
    }

        protected void finishPingingRound(PingingRound pingingRound) {
        pingingRound.close();
    }

    protected class PingingRound implements Releasable {
        private final int id;
        private final Map<TransportAddress, Connection> tempConnections = new HashMap<>();
        private final KeyedLock<TransportAddress> connectionLock = new KeyedLock<>(true);
        private final PingCollection pingCollection;
        private final List<DiscoveryNode> seedNodes;
        private final Consumer<PingCollection> pingListener;
        private final DiscoveryNode localNode;
        private final ConnectionProfile connectionProfile;

        private AtomicBoolean closed = new AtomicBoolean(false);

        PingingRound(int id, List<DiscoveryNode> seedNodes, Consumer<PingCollection> resultsConsumer, DiscoveryNode localNode,
                     ConnectionProfile connectionProfile) {
            this.id = id;
            this.seedNodes = Collections.unmodifiableList(new ArrayList<>(seedNodes));
            this.pingListener = resultsConsumer;
            this.localNode = localNode;
            this.connectionProfile = connectionProfile;
            this.pingCollection = new PingCollection();
        }

        public int id() {
            return this.id;
        }

        public boolean isClosed() {
            return this.closed.get();
        }

        public List<DiscoveryNode> getSeedNodes() {
            ensureOpen();
            return seedNodes;
        }

        public Connection getOrConnect(DiscoveryNode node) throws IOException {
            Connection result;
            try (Releasable ignore = connectionLock.acquire(node.getAddress())) {
                result = tempConnections.get(node.getAddress());
                if (result == null) {
                    ensureOpen();
                    boolean success = false;
                    logger.trace("[{}] opening connection to [{}]", id(), node);
                    result = transportService.openConnection(node, connectionProfile);
                    try {
                        transportService.handshake(result, connectionProfile.getHandshakeTimeout().millis());
                        synchronized (this) {
                                                        ensureOpen();
                            Connection existing = tempConnections.put(node.getAddress(), result);
                            assert existing == null;
                            success = true;
                        }
                    } finally {
                        if (success == false) {
                            logger.trace("[{}] closing connection to [{}] due to failure", id(), node);
                            IOUtils.closeWhileHandlingException(result);
                        }
                    }
                }
            }
            return result;
        }

        private void ensureOpen() {
            if (isClosed()) {
                throw new AlreadyClosedException("pinging round [" + id + "] is finished");
            }
        }

        public void addPingResponseToCollection(PingResponse pingResponse) {
            if (localNode.equals(pingResponse.node()) == false) {
                pingCollection.addPing(pingResponse);
            }
        }

        @Override
        public void close() {
            List<Connection> toClose = null;
            synchronized (this) {
                if (closed.compareAndSet(false, true)) {
                    activePingingRounds.remove(id);
                    toClose = new ArrayList<>(tempConnections.values());
                    tempConnections.clear();
                }
            }
            if (toClose != null) {
                                try {
                    pingListener.accept(pingCollection);
                } finally {
                    IOUtils.closeWhileHandlingException(toClose);
                }
            }
        }

        public ConnectionProfile getConnectionProfile() {
            return connectionProfile;
        }
    }


    protected void sendPings(final TimeValue timeout, final PingingRound pingingRound) {
        final ClusterState lastState = contextProvider.clusterState();
        final UnicastPingRequest pingRequest = new UnicastPingRequest(pingingRound.id(), timeout, createPingResponse(lastState));

        Set<DiscoveryNode> nodesFromResponses = temporalResponses.stream().map(pingResponse -> {
            assert clusterName.equals(pingResponse.clusterName()) :
                "got a ping request from a different cluster. expected " + clusterName + " got " + pingResponse.clusterName();
            return pingResponse.node();
        }).collect(Collectors.toSet());

                final Map<TransportAddress, DiscoveryNode> uniqueNodesByAddress =
            Stream.concat(pingingRound.getSeedNodes().stream(), nodesFromResponses.stream())
                .collect(Collectors.toMap(DiscoveryNode::getAddress, Function.identity(), (n1, n2) -> n1));


                final Set<DiscoveryNode> nodesToPing = uniqueNodesByAddress.values().stream()
            .map(node -> {
                DiscoveryNode foundNode = lastState.nodes().findByAddress(node.getAddress());
                if (foundNode == null) {
                    return node;
                } else {
                    return foundNode;
                }
            }).collect(Collectors.toSet());

        nodesToPing.forEach(node -> sendPingRequestToNode(node, timeout, pingingRound, pingRequest));
    }

    private void sendPingRequestToNode(final DiscoveryNode node, TimeValue timeout, final PingingRound pingingRound,
                                       final UnicastPingRequest pingRequest) {
        submitToExecutor(new AbstractRunnable() {
            @Override
            protected void doRun() throws Exception {
                Connection connection = null;
                if (transportService.nodeConnected(node)) {
                    try {
                                                connection = transportService.getConnection(node);
                    } catch (NodeNotConnectedException e) {
                        logger.trace("[{}] node [{}] just disconnected, will create a temp connection", pingingRound.id(), node);
                    }
                }

                if (connection == null) {
                    connection = pingingRound.getOrConnect(node);
                }

                logger.trace("[{}] sending to {}", pingingRound.id(), node);
                transportService.sendRequest(connection, ACTION_NAME, pingRequest,
                    TransportRequestOptions.builder().withTimeout((long) (timeout.millis() * 1.25)).build(),
                    getPingResponseHandler(pingingRound, node));
            }

            @Override
            public void onFailure(Exception e) {
                if (e instanceof ConnectTransportException || e instanceof AlreadyClosedException) {
                                        logger.trace(
                        (Supplier<?>) () -> new ParameterizedMessage(
                            "[{}] failed to ping {}", pingingRound.id(), node), e);
                } else if (e instanceof RemoteTransportException) {
                                        logger.debug(
                        (Supplier<?>) () -> new ParameterizedMessage(
                            "[{}] received a remote error as a response to ping {}", pingingRound.id(), node), e);
                } else {
                    logger.warn(
                        (Supplier<?>) () -> new ParameterizedMessage(
                            "[{}] failed send ping to {}", pingingRound.id(), node), e);
                }
            }

            @Override
            public void onRejection(Exception e) {
                                                logger.debug("Ping execution rejected", e);
            }
        });
    }

        protected void submitToExecutor(AbstractRunnable abstractRunnable) {
        unicastZenPingExecutorService.execute(abstractRunnable);
    }

        protected TransportResponseHandler<UnicastPingResponse> getPingResponseHandler(final PingingRound pingingRound,
                                                                                   final DiscoveryNode node) {
        return new TransportResponseHandler<UnicastPingResponse>() {

            @Override
            public UnicastPingResponse read(StreamInput in) throws IOException {
                return new UnicastPingResponse(in);
            }

            @Override
            public String executor() {
                return ThreadPool.Names.SAME;
            }

            @Override
            public void handleResponse(UnicastPingResponse response) {
                logger.trace("[{}] received response from {}: {}", pingingRound.id(), node, Arrays.toString(response.pingResponses));
                if (pingingRound.isClosed()) {
                    if (logger.isTraceEnabled()) {
                        logger.trace("[{}] skipping received response from {}. already closed", pingingRound.id(), node);
                    }
                } else {
                    Stream.of(response.pingResponses).forEach(pingingRound::addPingResponseToCollection);
                }
            }

            @Override
            public void handleException(TransportException exp) {
                if (exp instanceof ConnectTransportException || exp.getCause() instanceof ConnectTransportException ||
                    exp.getCause() instanceof AlreadyClosedException) {
                                        logger.trace((Supplier<?>) () -> new ParameterizedMessage("failed to connect to {}", node), exp);
                } else if (closed == false) {
                    logger.warn((Supplier<?>) () -> new ParameterizedMessage("failed to send ping to [{}]", node), exp);
                }
            }
        };
    }

    private UnicastPingResponse handlePingRequest(final UnicastPingRequest request) {
        assert clusterName.equals(request.pingResponse.clusterName()) :
            "got a ping request from a different cluster. expected " + clusterName + " got " + request.pingResponse.clusterName();
        temporalResponses.add(request.pingResponse);
                activePingingRounds.values().forEach(p -> p.addPingResponseToCollection(request.pingResponse));
        threadPool.schedule(TimeValue.timeValueMillis(request.timeout.millis() * 2), ThreadPool.Names.SAME,
            () -> temporalResponses.remove(request.pingResponse));

        List<PingResponse> pingResponses = CollectionUtils.iterableAsArrayList(temporalResponses);
        pingResponses.add(createPingResponse(contextProvider.clusterState()));

        return new UnicastPingResponse(request.id, pingResponses.toArray(new PingResponse[pingResponses.size()]));
    }

    class UnicastPingRequestHandler implements TransportRequestHandler<UnicastPingRequest> {

        @Override
        public void messageReceived(UnicastPingRequest request, TransportChannel channel) throws Exception {
            if (closed) {
                throw new AlreadyClosedException("node is shutting down");
            }
            if (request.pingResponse.clusterName().equals(clusterName)) {
                channel.sendResponse(handlePingRequest(request));
            } else {
                throw new IllegalStateException(
                    String.format(
                        Locale.ROOT,
                        "mismatched cluster names; request: [%s], local: [%s]",
                        request.pingResponse.clusterName().value(),
                        clusterName.value()));
            }
        }

    }

    static class UnicastPingRequest extends TransportRequest {

        final int id;
        final TimeValue timeout;
        final PingResponse pingResponse;

        UnicastPingRequest(int id, TimeValue timeout, PingResponse pingResponse) {
            this.id = id;
            this.timeout = timeout;
            this.pingResponse = pingResponse;
        }

        UnicastPingRequest(StreamInput in) throws IOException {
            super(in);
            id = in.readInt();
            timeout = new TimeValue(in);
            pingResponse = new PingResponse(in);
        }

        @Override
        public void readFrom(StreamInput in) throws IOException {
            throw new UnsupportedOperationException("usage of Streamable is to be replaced by Writeable");
        }

        @Override
        public void writeTo(StreamOutput out) throws IOException {
            super.writeTo(out);
            out.writeInt(id);
            timeout.writeTo(out);
            pingResponse.writeTo(out);
        }
    }

    private PingResponse createPingResponse(ClusterState clusterState) {
        DiscoveryNodes discoNodes = clusterState.nodes();
        return new PingResponse(discoNodes.getLocalNode(), discoNodes.getMasterNode(), clusterState);
    }

    static class UnicastPingResponse extends TransportResponse {

        final int id;

        final PingResponse[] pingResponses;

        UnicastPingResponse(int id, PingResponse[] pingResponses) {
            this.id = id;
            this.pingResponses = pingResponses;
        }

        UnicastPingResponse(StreamInput in) throws IOException {
            id = in.readInt();
            pingResponses = new PingResponse[in.readVInt()];
            for (int i = 0; i < pingResponses.length; i++) {
                pingResponses[i] = new PingResponse(in);
            }
        }

        @Override
        public void readFrom(StreamInput in) throws IOException {
            throw new UnsupportedOperationException("usage of Streamable is to be replaced by Writeable");
        }

        @Override
        public void writeTo(StreamOutput out) throws IOException {
            super.writeTo(out);
            out.writeInt(id);
            out.writeVInt(pingResponses.length);
            for (PingResponse pingResponse : pingResponses) {
                pingResponse.writeTo(out);
            }
        }
    }

    protected Version getVersion() {
        return Version.CURRENT;     }
}
