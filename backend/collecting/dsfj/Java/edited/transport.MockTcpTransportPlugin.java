
package org.elasticsearch.transport;

import org.elasticsearch.common.io.stream.NamedWriteableRegistry;
import org.elasticsearch.common.network.NetworkModule;
import org.elasticsearch.common.network.NetworkService;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.util.BigArrays;
import org.elasticsearch.common.util.PageCacheRecycler;
import org.elasticsearch.indices.breaker.CircuitBreakerService;
import org.elasticsearch.plugins.NetworkPlugin;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.threadpool.ThreadPool;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.function.Supplier;

public class MockTcpTransportPlugin extends Plugin implements NetworkPlugin {

    public static final String MOCK_TCP_TRANSPORT_NAME = "mock-socket-network";

    @Override
    public Map<String, Supplier<Transport>> getTransports(Settings settings, ThreadPool threadPool, BigArrays bigArrays,
                                                          PageCacheRecycler pageCacheRecycler,
                                                          CircuitBreakerService circuitBreakerService,
                                                          NamedWriteableRegistry namedWriteableRegistry,
                                                          NetworkService networkService) {
        return Collections.singletonMap(MOCK_TCP_TRANSPORT_NAME,
            () -> new MockTcpTransport(settings, threadPool, bigArrays, circuitBreakerService, namedWriteableRegistry, networkService));
    }
}
