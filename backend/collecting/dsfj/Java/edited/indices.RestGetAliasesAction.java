

package org.elasticsearch.rest.action.admin.indices;

import com.carrotsearch.hppc.cursors.ObjectObjectCursor;

import org.elasticsearch.action.admin.indices.alias.get.GetAliasesRequest;
import org.elasticsearch.action.admin.indices.alias.get.GetAliasesResponse;
import org.elasticsearch.action.support.IndicesOptions;
import org.elasticsearch.client.node.NodeClient;
import org.elasticsearch.cluster.metadata.AliasMetaData;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.collect.ImmutableOpenMap;
import org.elasticsearch.common.regex.Regex;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.util.set.Sets;
import org.elasticsearch.common.xcontent.ToXContent;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.rest.BaseRestHandler;
import org.elasticsearch.rest.BytesRestResponse;
import org.elasticsearch.rest.RestController;
import org.elasticsearch.rest.RestRequest;
import org.elasticsearch.rest.RestResponse;
import org.elasticsearch.rest.RestStatus;
import org.elasticsearch.rest.action.RestBuilderListener;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.SortedSet;
import java.util.stream.Collectors;

import static org.elasticsearch.rest.RestRequest.Method.GET;
import static org.elasticsearch.rest.RestRequest.Method.HEAD;


public class RestGetAliasesAction extends BaseRestHandler {

    public RestGetAliasesAction(final Settings settings, final RestController controller) {
        super(settings);
        controller.registerHandler(GET, "/_alias/{name}", this);
        controller.registerHandler(HEAD, "/_alias/{name}", this);
        controller.registerHandler(GET, "/{index}/_alias", this);
        controller.registerHandler(HEAD, "/{index}/_alias", this);
        controller.registerHandler(GET, "/{index}/_alias/{name}", this);
        controller.registerHandler(HEAD, "/{index}/_alias/{name}", this);
    }

    @Override
    public String getName() {
        return "get_aliases_action";
    }

    @Override
    public RestChannelConsumer prepareRequest(final RestRequest request, final NodeClient client) throws IOException {
        final boolean namesProvided = request.hasParam("name");
        final String[] aliases = request.paramAsStringArrayOrEmptyIfAll("name");
        final GetAliasesRequest getAliasesRequest = new GetAliasesRequest(aliases);
        final String[] indices = Strings.splitStringByCommaToArray(request.param("index"));
        getAliasesRequest.indices(indices);
        getAliasesRequest.indicesOptions(IndicesOptions.fromRequest(request, getAliasesRequest.indicesOptions()));
        getAliasesRequest.local(request.paramAsBoolean("local", getAliasesRequest.local()));

        return channel -> client.admin().indices().getAliases(getAliasesRequest, new RestBuilderListener<GetAliasesResponse>(channel) {
            @Override
            public RestResponse buildResponse(GetAliasesResponse response, XContentBuilder builder) throws Exception {
                final ImmutableOpenMap<String, List<AliasMetaData>> aliasMap = response.getAliases();

                final Set<String> aliasNames = new HashSet<>();
                final Set<String> indicesToDisplay = new HashSet<>();
                for (final ObjectObjectCursor<String, List<AliasMetaData>> cursor : aliasMap) {
                    for (final AliasMetaData aliasMetaData : cursor.value) {
                        aliasNames.add(aliasMetaData.alias());
                        if (namesProvided) {
                            indicesToDisplay.add(cursor.key);
                        }
                    }
                }

                                final SortedSet<String> difference = Sets.sortedDifference(Arrays.stream(aliases).collect(Collectors.toSet()), aliasNames);

                                final List<String> matches = new ArrayList<>();
                outer:
                for (final String pattern : difference) {
                    if (pattern.contains("*")) {
                        for (final String aliasName : aliasNames) {
                            if (Regex.simpleMatch(pattern, aliasName)) {
                                matches.add(pattern);
                                continue outer;
                            }
                        }
                    }
                }
                difference.removeAll(matches);

                final RestStatus status;
                builder.startObject();
                {
                    if (difference.isEmpty()) {
                        status = RestStatus.OK;
                    } else {
                        status = RestStatus.NOT_FOUND;
                        final String message;
                        if (difference.size() == 1) {
                            message = String.format(Locale.ROOT, "alias [%s] missing", toNamesString(difference.iterator().next()));
                        } else {
                            message = String.format(Locale.ROOT, "aliases [%s] missing", toNamesString(difference.toArray(new String[0])));
                        }
                        builder.field("error", message);
                        builder.field("status", status.getStatus());
                    }

                    for (final ObjectObjectCursor<String, List<AliasMetaData>> entry : response.getAliases()) {
                        if (namesProvided == false || (namesProvided && indicesToDisplay.contains(entry.key))) {
                            builder.startObject(entry.key);
                            {
                                builder.startObject("aliases");
                                {
                                    for (final AliasMetaData alias : entry.value) {
                                        AliasMetaData.Builder.toXContent(alias, builder, ToXContent.EMPTY_PARAMS);
                                    }
                                }
                                builder.endObject();
                            }
                            builder.endObject();
                        }
                    }
                }
                builder.endObject();
                return new BytesRestResponse(status, builder);
            }

        });
    }

    private static String toNamesString(final String... names) {
        if (names == null || names.length == 0) {
            return "";
        } else if (names.length == 1) {
            return names[0];
        } else {
            return Arrays.stream(names).collect(Collectors.joining(","));
        }
    }

}
