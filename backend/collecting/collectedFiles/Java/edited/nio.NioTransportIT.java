
package org.elasticsearch.transport.nio;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.NioIntegTestCase;
import org.elasticsearch.Version;
import org.elasticsearch.action.admin.cluster.health.ClusterHealthResponse;
import org.elasticsearch.client.Client;
import org.elasticsearch.cluster.health.ClusterHealthStatus;
import org.elasticsearch.common.io.stream.NamedWriteableRegistry;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.network.NetworkModule;
import org.elasticsearch.common.network.NetworkService;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.util.BigArrays;
import org.elasticsearch.common.util.PageCacheRecycler;
import org.elasticsearch.indices.breaker.CircuitBreakerService;
import org.elasticsearch.plugins.NetworkPlugin;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.test.ESIntegTestCase.ClusterScope;
import org.elasticsearch.test.ESIntegTestCase.Scope;
import org.elasticsearch.threadpool.ThreadPool;
import org.elasticsearch.transport.TcpChannel;
import org.elasticsearch.transport.TcpTransport;
import org.elasticsearch.transport.Transport;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.function.Supplier;

import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.is;

@ClusterScope(scope = Scope.TEST, supportsDedicatedMasters = false, numDataNodes = 1)
public class NioTransportIT extends NioIntegTestCase {
        private static String channelProfileName = null;

    @Override
    protected Settings nodeSettings(int nodeOrdinal) {
        return Settings.builder().put(super.nodeSettings(nodeOrdinal))
                .put(NetworkModule.TRANSPORT_TYPE_KEY, "exception-throwing").build();
    }

    @Override
    protected Collection<Class<? extends Plugin>> nodePlugins() {
        List<Class<? extends Plugin>> list = new ArrayList<>();
        list.add(ExceptionThrowingNioTransport.TestPlugin.class);
        list.addAll(super.nodePlugins());
        return Collections.unmodifiableCollection(list);
    }

    public void testThatConnectionFailsAsIntended() throws Exception {
        Client transportClient = internalCluster().transportClient();
        ClusterHealthResponse clusterIndexHealths = transportClient.admin().cluster().prepareHealth().get();
        assertThat(clusterIndexHealths.getStatus(), is(ClusterHealthStatus.GREEN));
        try {
            transportClient.filterWithHeader(Collections.singletonMap("ERROR", "MY MESSAGE")).admin().cluster().prepareHealth().get();
            fail("Expected exception, but didn't happen");
        } catch (ElasticsearchException e) {
            assertThat(e.getMessage(), containsString("MY MESSAGE"));
            assertThat(channelProfileName, is(TcpTransport.DEFAULT_PROFILE));
        }
    }

    public static final class ExceptionThrowingNioTransport extends NioTransport {

        public static class TestPlugin extends Plugin implements NetworkPlugin {

            @Override
            public Map<String, Supplier<Transport>> getTransports(Settings settings, ThreadPool threadPool, BigArrays bigArrays,
                                                                  PageCacheRecycler pageCacheRecycler,
                                                                  CircuitBreakerService circuitBreakerService,
                                                                  NamedWriteableRegistry namedWriteableRegistry,
                                                                  NetworkService networkService) {
                return Collections.singletonMap("exception-throwing",
                    () -> new ExceptionThrowingNioTransport(settings, threadPool, networkService, bigArrays, pageCacheRecycler,
                        namedWriteableRegistry, circuitBreakerService));
            }
        }

        ExceptionThrowingNioTransport(Settings settings, ThreadPool threadPool, NetworkService networkService, BigArrays bigArrays,
                                      PageCacheRecycler pageCacheRecycler, NamedWriteableRegistry namedWriteableRegistry,
                                      CircuitBreakerService circuitBreakerService) {
            super(settings, threadPool, networkService, bigArrays, pageCacheRecycler, namedWriteableRegistry, circuitBreakerService);
        }

        @Override
        protected String handleRequest(TcpChannel channel, String profileName,
                                       StreamInput stream, long requestId, int messageLengthBytes, Version version,
                                       InetSocketAddress remoteAddress, byte status) throws IOException {
            String action = super.handleRequest(channel, profileName, stream, requestId, messageLengthBytes, version,
                    remoteAddress, status);
            channelProfileName = TcpTransport.DEFAULT_PROFILE;
            return action;
        }

        @Override
        protected void validateRequest(StreamInput buffer, long requestId, String action)
                throws IOException {
            super.validateRequest(buffer, requestId, action);
            String error = threadPool.getThreadContext().getHeader("ERROR");
            if (error != null) {
                throw new ElasticsearchException(error);
            }
        }

    }
}
