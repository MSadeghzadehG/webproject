

package org.elasticsearch.plugin.discovery.gce;

import com.google.api.client.http.HttpHeaders;
import com.google.api.client.util.ClassInfo;
import org.apache.logging.log4j.Logger;
import org.elasticsearch.core.internal.io.IOUtils;
import org.apache.lucene.util.SetOnce;
import org.elasticsearch.cloud.gce.GceInstancesService;
import org.elasticsearch.cloud.gce.GceInstancesServiceImpl;
import org.elasticsearch.cloud.gce.GceMetadataService;
import org.elasticsearch.cloud.gce.network.GceNameResolver;
import org.elasticsearch.cloud.gce.util.Access;
import org.elasticsearch.common.logging.Loggers;
import org.elasticsearch.common.network.NetworkService;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.discovery.gce.GceUnicastHostsProvider;
import org.elasticsearch.discovery.zen.UnicastHostsProvider;
import org.elasticsearch.plugins.DiscoveryPlugin;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.transport.TransportService;

import java.io.Closeable;
import java.io.IOException;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.function.Supplier;

public class GceDiscoveryPlugin extends Plugin implements DiscoveryPlugin, Closeable {

    public static final String GCE = "gce";
    private final Settings settings;
    private static final Logger logger = Loggers.getLogger(GceDiscoveryPlugin.class);
        private final SetOnce<GceInstancesService> gceInstancesService = new SetOnce<>();

    static {
        
        Access.doPrivilegedVoid( () -> ClassInfo.of(HttpHeaders.class, true));
    }

    public GceDiscoveryPlugin(Settings settings) {
        this.settings = settings;
        logger.trace("starting gce discovery plugin...");
    }

        protected GceInstancesService createGceInstancesService() {
        return new GceInstancesServiceImpl(settings);
    }

    @Override
    public Map<String, Supplier<UnicastHostsProvider>> getZenHostsProviders(TransportService transportService,
                                                                            NetworkService networkService) {
        return Collections.singletonMap(GCE, () -> {
            gceInstancesService.set(createGceInstancesService());
            return new GceUnicastHostsProvider(settings, gceInstancesService.get(), transportService, networkService);
        });
    }

    @Override
    public NetworkService.CustomNameResolver getCustomNameResolver(Settings settings) {
        logger.debug("Register _gce_, _gce:xxx network names");
        return new GceNameResolver(settings, new GceMetadataService(settings));
    }

    @Override
    public List<Setting<?>> getSettings() {
        return Arrays.asList(
                        GceInstancesService.PROJECT_SETTING,
            GceInstancesService.ZONE_SETTING,
            GceUnicastHostsProvider.TAGS_SETTING,
            GceInstancesService.REFRESH_SETTING,
            GceInstancesService.RETRY_SETTING,
            GceInstancesService.MAX_WAIT_SETTING);
    }



    @Override
    public void close() throws IOException {
        IOUtils.close(gceInstancesService.get());
    }
}
