
package org.elasticsearch.transport;

import org.elasticsearch.client.transport.TransportClient;
import org.elasticsearch.common.io.stream.NamedWriteableRegistry;
import org.elasticsearch.common.network.NetworkModule;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.transport.nio.MockNioTransportPlugin;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;

@SuppressWarnings({"unchecked","varargs"})
@Deprecated
public class MockTransportClient extends TransportClient {
    private static final Settings DEFAULT_SETTINGS = Settings.builder().put("transport.type.default",
        MockTcpTransportPlugin.MOCK_TCP_TRANSPORT_NAME).build();

    public MockTransportClient(Settings settings, Class<? extends Plugin>... plugins) {
        this(settings, Arrays.asList(plugins));
    }

    public MockTransportClient(Settings settings, Collection<Class<? extends Plugin>> plugins) {
        this(settings, plugins, null);
    }

    public MockTransportClient(Settings settings, Collection<Class<? extends Plugin>> plugins, HostFailureListener listener) {
        super(settings, DEFAULT_SETTINGS, addMockTransportIfMissing(settings, plugins), listener);
    }

    private static Collection<Class<? extends Plugin>> addMockTransportIfMissing(Settings settings,
                                                                                 Collection<Class<? extends Plugin>> plugins) {
        boolean settingExists = NetworkModule.TRANSPORT_TYPE_SETTING.exists(settings);
        String transportType = NetworkModule.TRANSPORT_TYPE_SETTING.get(settings);
        if (settingExists == false || MockTcpTransportPlugin.MOCK_TCP_TRANSPORT_NAME.equals(transportType)) {
            if (plugins.contains(MockTcpTransportPlugin.class)) {
                return plugins;
            } else {
                plugins = new ArrayList<>(plugins);
                plugins.add(MockTcpTransportPlugin.class);
                return plugins;
            }
        } else if (MockNioTransportPlugin.MOCK_NIO_TRANSPORT_NAME.equals(transportType)) {
            if (plugins.contains(MockNioTransportPlugin.class)) {
                return plugins;
            } else {
                plugins = new ArrayList<>(plugins);
                plugins.add(MockNioTransportPlugin.class);
                return plugins;
            }
        }
        return plugins;
    }

    public NamedWriteableRegistry getNamedWriteableRegistry() {
        return namedWriteableRegistry;
    }
}
