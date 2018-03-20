

package org.elasticsearch.action.search;

import org.apache.logging.log4j.Logger;
import org.elasticsearch.action.ActionListener;
import org.elasticsearch.cluster.routing.GroupShardsIterator;
import org.elasticsearch.cluster.routing.ShardRouting;
import org.elasticsearch.search.dfs.DfsSearchResult;
import org.elasticsearch.search.internal.AliasFilter;
import org.elasticsearch.transport.Transport;

import java.util.Map;
import java.util.concurrent.Executor;
import java.util.function.BiFunction;

final class SearchDfsQueryThenFetchAsyncAction extends AbstractSearchAsyncAction<DfsSearchResult> {

    private final SearchPhaseController searchPhaseController;

    SearchDfsQueryThenFetchAsyncAction(final Logger logger, final SearchTransportService searchTransportService,
            final BiFunction<String, String, Transport.Connection> nodeIdToConnection, final Map<String, AliasFilter> aliasFilter,
            final Map<String, Float> concreteIndexBoosts, final SearchPhaseController searchPhaseController, final Executor executor,
            final SearchRequest request, final ActionListener<SearchResponse> listener,
            final GroupShardsIterator<SearchShardIterator> shardsIts, final TransportSearchAction.SearchTimeProvider timeProvider,
            final long clusterStateVersion, final SearchTask task, SearchResponse.Clusters clusters) {
        super("dfs", logger, searchTransportService, nodeIdToConnection, aliasFilter, concreteIndexBoosts, executor, request, listener,
                shardsIts, timeProvider, clusterStateVersion, task, new ArraySearchPhaseResults<>(shardsIts.size()),
                request.getMaxConcurrentShardRequests(), clusters);
        this.searchPhaseController = searchPhaseController;
    }

    @Override
    protected void executePhaseOnShard(final SearchShardIterator shardIt, final ShardRouting shard,
                                       final SearchActionListener<DfsSearchResult> listener) {
        getSearchTransport().sendExecuteDfs(getConnection(shardIt.getClusterAlias(), shard.currentNodeId()),
            buildShardSearchRequest(shardIt) , getTask(), listener);
    }

    @Override
    protected SearchPhase getNextPhase(final SearchPhaseResults<DfsSearchResult> results, final SearchPhaseContext context) {
        return new DfsQueryPhase(results.getAtomicArray(), searchPhaseController, (queryResults) ->
            new FetchSearchPhase(queryResults, searchPhaseController, context), context);
    }
}
