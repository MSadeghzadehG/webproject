

package org.elasticsearch.index.reindex;

import org.elasticsearch.action.search.SearchRequest;
import org.elasticsearch.client.node.NodeClient;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.rest.RestController;
import org.elasticsearch.rest.RestRequest;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.function.Consumer;

import static org.elasticsearch.rest.RestRequest.Method.POST;

public class RestDeleteByQueryAction extends AbstractBulkByQueryRestHandler<DeleteByQueryRequest, DeleteByQueryAction> {
    public RestDeleteByQueryAction(Settings settings, RestController controller) {
        super(settings, DeleteByQueryAction.INSTANCE);
        controller.registerHandler(POST, "/{index}/_delete_by_query", this);
        controller.registerHandler(POST, "/{index}/{type}/_delete_by_query", this);
    }

    @Override
    public String getName() {
        return "delete_by_query_action";
    }

    @Override
    public RestChannelConsumer prepareRequest(RestRequest request, NodeClient client) throws IOException {
        return doPrepareRequest(request, client, false, false);
    }

    @Override
    protected DeleteByQueryRequest buildRequest(RestRequest request) throws IOException {
        
        DeleteByQueryRequest internal = new DeleteByQueryRequest(new SearchRequest());

        Map<String, Consumer<Object>> consumers = new HashMap<>();
        consumers.put("conflicts", o -> internal.setConflicts((String) o));

        parseInternalRequest(internal, request, consumers);

        return internal;
    }
}
