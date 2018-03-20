

package org.elasticsearch.index.rankeval;

import org.elasticsearch.client.node.NodeClient;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.xcontent.XContentParser;
import org.elasticsearch.rest.BaseRestHandler;
import org.elasticsearch.rest.RestController;
import org.elasticsearch.rest.RestRequest;
import org.elasticsearch.rest.action.RestToXContentListener;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;

import static org.elasticsearch.rest.RestRequest.Method.GET;
import static org.elasticsearch.rest.RestRequest.Method.POST;


public class RestRankEvalAction extends BaseRestHandler {

    public static String ENDPOINT = "_rank_eval";

    public RestRankEvalAction(Settings settings, RestController controller) {
        super(settings);
        controller.registerHandler(GET, "/" + ENDPOINT, this);
        controller.registerHandler(POST, "/" + ENDPOINT, this);
        controller.registerHandler(GET, "/{index}/" + ENDPOINT, this);
        controller.registerHandler(POST, "/{index}/" + ENDPOINT, this);
    }

    @Override
    protected RestChannelConsumer prepareRequest(RestRequest request, NodeClient client) throws IOException {
        RankEvalRequest rankEvalRequest = new RankEvalRequest();
        try (XContentParser parser = request.contentOrSourceParamParser()) {
            parseRankEvalRequest(rankEvalRequest, request, parser);
        }
        return channel -> client.executeLocally(RankEvalAction.INSTANCE, rankEvalRequest,
                new RestToXContentListener<RankEvalResponse>(channel));
    }

    private static void parseRankEvalRequest(RankEvalRequest rankEvalRequest, RestRequest request, XContentParser parser) {
        List<String> indices = Arrays.asList(Strings.splitStringByCommaToArray(request.param("index")));
        RankEvalSpec spec = RankEvalSpec.parse(parser);
        spec.addIndices(indices);
        rankEvalRequest.setRankEvalSpec(spec);
    }

    @Override
    public String getName() {
        return "rank_eval_action";
    }
}
