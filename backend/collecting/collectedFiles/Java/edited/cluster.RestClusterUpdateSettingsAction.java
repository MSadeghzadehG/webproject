

package org.elasticsearch.rest.action.admin.cluster;

import org.elasticsearch.action.admin.cluster.settings.ClusterUpdateSettingsRequest;
import org.elasticsearch.client.Requests;
import org.elasticsearch.client.node.NodeClient;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.xcontent.XContentParser;
import org.elasticsearch.rest.BaseRestHandler;
import org.elasticsearch.rest.RestController;
import org.elasticsearch.rest.RestRequest;
import org.elasticsearch.rest.action.RestToXContentListener;

import java.io.IOException;
import java.util.Map;
import java.util.Set;

public class RestClusterUpdateSettingsAction extends BaseRestHandler {

    private static final String PERSISTENT = "persistent";
    private static final String TRANSIENT = "transient";

    public RestClusterUpdateSettingsAction(Settings settings, RestController controller) {
        super(settings);
        controller.registerHandler(RestRequest.Method.PUT, "/_cluster/settings", this);
    }

    @Override
    public String getName() {
        return "cluster_update_settings_action";
    }

    @Override
    public RestChannelConsumer prepareRequest(final RestRequest request, final NodeClient client) throws IOException {
        final ClusterUpdateSettingsRequest clusterUpdateSettingsRequest = Requests.clusterUpdateSettingsRequest();
        clusterUpdateSettingsRequest.timeout(request.paramAsTime("timeout", clusterUpdateSettingsRequest.timeout()));
        clusterUpdateSettingsRequest.masterNodeTimeout(
                request.paramAsTime("master_timeout", clusterUpdateSettingsRequest.masterNodeTimeout()));
        Map<String, Object> source;
        try (XContentParser parser = request.contentParser()) {
            source = parser.map();
        }
        if (source.containsKey(TRANSIENT)) {
            clusterUpdateSettingsRequest.transientSettings((Map) source.get(TRANSIENT));
        }
        if (source.containsKey(PERSISTENT)) {
            clusterUpdateSettingsRequest.persistentSettings((Map) source.get(PERSISTENT));
        }

        return channel -> client.admin().cluster().updateSettings(clusterUpdateSettingsRequest, new RestToXContentListener<>(channel));
    }

    @Override
    protected Set<String> responseParams() {
        return Settings.FORMAT_PARAMS;
    }

    @Override
    public boolean canTripCircuitBreaker() {
        return false;
    }
}
