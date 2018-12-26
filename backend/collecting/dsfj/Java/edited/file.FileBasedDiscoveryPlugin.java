

package org.elasticsearch.discovery.file;

import org.apache.logging.log4j.Logger;
import org.elasticsearch.client.Client;
import org.elasticsearch.cluster.service.ClusterService;
import org.elasticsearch.common.io.stream.NamedWriteableRegistry;
import org.elasticsearch.common.logging.Loggers;
import org.elasticsearch.common.network.NetworkService;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.util.concurrent.EsExecutors;
import org.elasticsearch.common.xcontent.NamedXContentRegistry;
import org.elasticsearch.discovery.zen.UnicastHostsProvider;
import org.elasticsearch.discovery.zen.UnicastZenPing;
import org.elasticsearch.env.Environment;
import org.elasticsearch.env.NodeEnvironment;
import org.elasticsearch.node.Node;
import org.elasticsearch.plugins.DiscoveryPlugin;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.script.ScriptService;
import org.elasticsearch.threadpool.ThreadPool;
import org.elasticsearch.transport.TransportService;
import org.elasticsearch.watcher.ResourceWatcherService;

import java.io.IOException;
import java.nio.file.Path;
import java.util.Collection;
import java.util.Collections;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.function.Supplier;


public class FileBasedDiscoveryPlugin extends Plugin implements DiscoveryPlugin {

    private static final Logger logger = Loggers.getLogger(FileBasedDiscoveryPlugin.class);

    private final Settings settings;
    private final Path configPath;
    private ExecutorService fileBasedDiscoveryExecutorService;

    public FileBasedDiscoveryPlugin(Settings settings, Path configPath) {
        this.settings = settings;
        this.configPath = configPath;
    }

    @Override
    public Collection<Object> createComponents(Client client, ClusterService clusterService, ThreadPool threadPool,
                                               ResourceWatcherService resourceWatcherService, ScriptService scriptService,
                                               NamedXContentRegistry xContentRegistry, Environment environment,
                                               NodeEnvironment nodeEnvironment, NamedWriteableRegistry namedWriteableRegistry) {
        final int concurrentConnects = UnicastZenPing.DISCOVERY_ZEN_PING_UNICAST_CONCURRENT_CONNECTS_SETTING.get(settings);
        final ThreadFactory threadFactory = EsExecutors.daemonThreadFactory(settings, "[file_based_discovery_resolve]");
        fileBasedDiscoveryExecutorService = EsExecutors.newScaling(
                Node.NODE_NAME_SETTING.get(settings) + "/" + "file_based_discovery_resolve",
                0,
                concurrentConnects,
                60,
                TimeUnit.SECONDS,
                threadFactory,
                threadPool.getThreadContext());

        return Collections.emptyList();
    }

    @Override
    public void close() throws IOException {
        ThreadPool.terminate(fileBasedDiscoveryExecutorService, 0, TimeUnit.SECONDS);
    }

    @Override
    public Map<String, Supplier<UnicastHostsProvider>> getZenHostsProviders(TransportService transportService,
                                                                            NetworkService networkService) {
        return Collections.singletonMap(
            "file",
            () -> new FileBasedUnicastHostsProvider(
                    new Environment(settings, configPath), transportService, fileBasedDiscoveryExecutorService));
    }
}
