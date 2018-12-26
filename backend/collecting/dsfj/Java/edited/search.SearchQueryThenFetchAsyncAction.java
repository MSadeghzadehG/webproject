

package org.elasticsearch.action.search;

import org.apache.logging.log4j.Logger;
import org.elasticsearch.action.ActionListener;
import org.elasticsearch.cluster.routing.GroupShardsIterator;
import org.elasticsearch.cluster.routing.ShardRouting;
import org.elasticsearch.search.SearchPhaseResult;
import org.elasticsearch.search.internal.AliasFilter;
import org.elasticsearch.transport.Transport;

import java.util.Map;
import java.util.concurrent.Executor;
import java.util.function.BiFunction;

final class SearchQueryThenFetchAsyncAction extends AbstractSearchAsyncAction<SearchPhaseResult> {

    private final SearchPhaseController searchPhaseController;

    SearchQueryThenFetchAsyncAction(final Logger logger, final SearchTransportService searchTransportService,
            final BiFunction<String, String, Transport.Connection> nodeIdToConnection, final Map<String, AliasFilter> aliasFilter,
            final Map<String, Float> concreteIndexBoosts, final SearchPhaseController searchPhaseController, final Executor executor,
            final SearchRequest request, final ActionListener<SearchResponse> listener,
            final GroupShardsIterator<SearchShardIterator> shardsIts, final TransportSearchAction.SearchTimeProvider timeProvider,
            long clusterStateVersion, SearchTask task, SearchResponse.Clusters clusters) {
        super("query", logger, searchTransportService, nodeIdToConnection, aliasFilter, concreteIndexBoosts, executor, request, listener,
            shardsIts, timeProvider, clusterStateVersion, task, searchPhaseController.newSearchPhaseResults(request, shardsIts.size()),
                request.getMaxConcurrentShardRequests(), clusters);
        this.searchPhaseController = searchPhaseController;
    }

    protected void executePhaseOnShard(final SearchShardIterator shardIt, final ShardRouting shard,
                                       final SearchActionListener<SearchPhaseResult> listener) {
        getSearchTransport().sendExecuteQuery(getConnection(shardIt.getClusterAlias(), shard.currentNodeId()),
            buildShardSearchRequest(shardIt), getTask(), listener);
    }

    @Override
    protected SearchPhase getNextPhase(final SearchPhaseResults<SearchPhaseResult> results, final SearchPhaseContext context) {
        return new FetchSearchPhase(results, searchPhaseController, context);
    }
}
