
package org.elasticsearch.action.search;

import org.apache.logging.log4j.Logger;
import org.elasticsearch.action.OriginalIndices;
import org.elasticsearch.common.Nullable;
import org.elasticsearch.common.logging.Loggers;
import org.elasticsearch.search.SearchShardTarget;
import org.elasticsearch.search.internal.InternalSearchResponse;
import org.elasticsearch.search.internal.ShardSearchTransportRequest;
import org.elasticsearch.transport.Transport;
import org.junit.Assert;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;


public final class MockSearchPhaseContext implements SearchPhaseContext {
    private static final Logger logger = Loggers.getLogger(MockSearchPhaseContext.class);
    public AtomicReference<Throwable> phaseFailure = new AtomicReference<>();
    final int numShards;
    final AtomicInteger numSuccess;
    List<ShardSearchFailure> failures = Collections.synchronizedList(new ArrayList<>());
    SearchTransportService searchTransport;
    Set<Long> releasedSearchContexts = new HashSet<>();
    SearchRequest searchRequest = new SearchRequest();
    AtomicInteger phasesExecuted = new AtomicInteger();

    public MockSearchPhaseContext(int numShards) {
        this.numShards = numShards;
        numSuccess = new AtomicInteger(numShards);
    }

    public void assertNoFailure() {
        if (phaseFailure.get() != null) {
            throw new AssertionError(phaseFailure.get());
        }
    }

    @Override
    public int getNumShards() {
        return numShards;
    }

    @Override
    public Logger getLogger() {
        return logger;
    }

    @Override
    public SearchTask getTask() {
        return new SearchTask(0, "n/a", "n/a", "test", null, Collections.emptyMap());
    }

    @Override
    public SearchRequest getRequest() {
        return searchRequest;
    }

    @Override
    public SearchResponse buildSearchResponse(InternalSearchResponse internalSearchResponse, String scrollId) {
        return new SearchResponse(internalSearchResponse, scrollId, numShards, numSuccess.get(), 0, 0,
            failures.toArray(new ShardSearchFailure[failures.size()]), SearchResponse.Clusters.EMPTY);
    }

    @Override
    public void onPhaseFailure(SearchPhase phase, String msg, Throwable cause) {
        phaseFailure.set(cause);
    }

    @Override
    public void onShardFailure(int shardIndex, @Nullable SearchShardTarget shardTarget, Exception e) {
        failures.add(new ShardSearchFailure(e, shardTarget));
        numSuccess.decrementAndGet();
    }

    @Override
    public Transport.Connection getConnection(String clusterAlias, String nodeId) {
        return null;     }

    @Override
    public SearchTransportService getSearchTransport() {
        Assert.assertNotNull(searchTransport);
        return searchTransport;
    }

    @Override
    public ShardSearchTransportRequest buildShardSearchRequest(SearchShardIterator shardIt) {
        Assert.fail("should not be called");
        return null;
    }

    @Override
    public void executeNextPhase(SearchPhase currentPhase, SearchPhase nextPhase) {
        phasesExecuted.incrementAndGet();
        try {
            nextPhase.run();
        } catch (Exception e) {
           onPhaseFailure(nextPhase, "phase failed", e);
        }
    }

    @Override
    public void execute(Runnable command) {
        command.run();
    }

    @Override
    public void onResponse(SearchResponse response) {
        Assert.fail("should not be called");
    }

    @Override
    public void onFailure(Exception e) {
        Assert.fail("should not be called");
    }

    @Override
    public void sendReleaseSearchContext(long contextId, Transport.Connection connection, OriginalIndices originalIndices) {
        releasedSearchContexts.add(contextId);
    }
}
