

package org.elasticsearch.discovery.file;

import org.elasticsearch.cluster.node.DiscoveryNode;
import org.elasticsearch.common.io.stream.NamedWriteableRegistry;
import org.elasticsearch.common.network.NetworkService;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.transport.BoundTransportAddress;
import org.elasticsearch.common.transport.TransportAddress;
import org.elasticsearch.common.util.BigArrays;
import org.elasticsearch.env.Environment;
import org.elasticsearch.env.TestEnvironment;
import org.elasticsearch.indices.breaker.NoneCircuitBreakerService;
import org.elasticsearch.test.ESTestCase;
import org.elasticsearch.test.transport.MockTransportService;
import org.elasticsearch.threadpool.TestThreadPool;
import org.elasticsearch.threadpool.ThreadPool;
import org.elasticsearch.transport.MockTcpTransport;
import org.elasticsearch.transport.TransportService;
import org.junit.After;
import org.junit.Before;

import java.io.BufferedWriter;
import java.io.IOException;
import java.net.InetAddress;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static org.elasticsearch.discovery.file.FileBasedUnicastHostsProvider.UNICAST_HOSTS_FILE;
import static org.elasticsearch.discovery.file.FileBasedUnicastHostsProvider.UNICAST_HOST_PREFIX;


public class FileBasedUnicastHostsProviderTests extends ESTestCase {

    private ThreadPool threadPool;
    private ExecutorService executorService;
    private MockTransportService transportService;

    @Before
    public void setUp() throws Exception {
        super.setUp();
        threadPool = new TestThreadPool(FileBasedUnicastHostsProviderTests.class.getName());
        executorService = Executors.newSingleThreadExecutor();
    }

    @After
    public void tearDown() throws Exception {
        try {
            terminate(executorService);
        } finally {
            try {
                terminate(threadPool);
            } finally {
                super.tearDown();
            }
        }
    }

    @Before
    public void createTransportSvc() {
        MockTcpTransport transport =
            new MockTcpTransport(Settings.EMPTY,
                                    threadPool,
                                    BigArrays.NON_RECYCLING_INSTANCE,
                                    new NoneCircuitBreakerService(),
                                    new NamedWriteableRegistry(Collections.emptyList()),
                                    new NetworkService(Collections.emptyList())) {
                @Override
                public BoundTransportAddress boundAddress() {
                    return new BoundTransportAddress(
                        new TransportAddress[]{new TransportAddress(InetAddress.getLoopbackAddress(), 9300)},
                        new TransportAddress(InetAddress.getLoopbackAddress(), 9300)
                    );
                }
            };
        transportService = new MockTransportService(Settings.EMPTY, transport, threadPool, TransportService.NOOP_TRANSPORT_INTERCEPTOR,
                null);
    }

    public void testBuildDynamicNodes() throws Exception {
        final List<String> hostEntries = Arrays.asList("#comment, should be ignored", "192.168.0.1", "192.168.0.2:9305", "255.255.23.15");
        final List<DiscoveryNode> nodes = setupAndRunHostProvider(hostEntries);
        assertEquals(hostEntries.size() - 1, nodes.size());         assertEquals("192.168.0.1", nodes.get(0).getAddress().getAddress());
        assertEquals(9300, nodes.get(0).getAddress().getPort());
        assertEquals(UNICAST_HOST_PREFIX + "192.168.0.1_0#", nodes.get(0).getId());
        assertEquals("192.168.0.2", nodes.get(1).getAddress().getAddress());
        assertEquals(9305, nodes.get(1).getAddress().getPort());
        assertEquals(UNICAST_HOST_PREFIX + "192.168.0.2:9305_0#", nodes.get(1).getId());
        assertEquals("255.255.23.15", nodes.get(2).getAddress().getAddress());
        assertEquals(9300, nodes.get(2).getAddress().getPort());
        assertEquals(UNICAST_HOST_PREFIX + "255.255.23.15_0#", nodes.get(2).getId());
    }

    public void testEmptyUnicastHostsFile() throws Exception {
        final List<String> hostEntries = Collections.emptyList();
        final List<DiscoveryNode> nodes = setupAndRunHostProvider(hostEntries);
        assertEquals(0, nodes.size());
    }

    public void testUnicastHostsDoesNotExist() throws Exception {
        final Settings settings = Settings.builder()
                                      .put(Environment.PATH_HOME_SETTING.getKey(), createTempDir())
                                      .build();
        final Environment environment = TestEnvironment.newEnvironment(settings);
        final FileBasedUnicastHostsProvider provider = new FileBasedUnicastHostsProvider(environment, transportService, executorService);
        final List<DiscoveryNode> nodes = provider.buildDynamicNodes();
        assertEquals(0, nodes.size());
    }

    public void testInvalidHostEntries() throws Exception {
        List<String> hostEntries = Arrays.asList("192.168.0.1:9300:9300");
        List<DiscoveryNode> nodes = setupAndRunHostProvider(hostEntries);
        assertEquals(0, nodes.size());
    }

    public void testSomeInvalidHostEntries() throws Exception {
        List<String> hostEntries = Arrays.asList("192.168.0.1:9300:9300", "192.168.0.1:9301");
        List<DiscoveryNode> nodes = setupAndRunHostProvider(hostEntries);
        assertEquals(1, nodes.size());         assertEquals("192.168.0.1", nodes.get(0).getAddress().getAddress());
        assertEquals(9301, nodes.get(0).getAddress().getPort());
    }

            private List<DiscoveryNode> setupAndRunHostProvider(final List<String> hostEntries) throws IOException {
        final Path homeDir = createTempDir();
        final Settings settings = Settings.builder()
                                      .put(Environment.PATH_HOME_SETTING.getKey(), homeDir)
                                      .build();
        final Path configPath;
        if (randomBoolean()) {
            configPath = homeDir.resolve("config");
        } else {
            configPath = createTempDir();
        }
        final Path discoveryFilePath = configPath.resolve("discovery-file");
        Files.createDirectories(discoveryFilePath);
        final Path unicastHostsPath = discoveryFilePath.resolve(UNICAST_HOSTS_FILE);
        try (BufferedWriter writer = Files.newBufferedWriter(unicastHostsPath)) {
            writer.write(String.join("\n", hostEntries));
        }

        return new FileBasedUnicastHostsProvider(
                new Environment(settings, configPath), transportService, executorService).buildDynamicNodes();
    }
}
