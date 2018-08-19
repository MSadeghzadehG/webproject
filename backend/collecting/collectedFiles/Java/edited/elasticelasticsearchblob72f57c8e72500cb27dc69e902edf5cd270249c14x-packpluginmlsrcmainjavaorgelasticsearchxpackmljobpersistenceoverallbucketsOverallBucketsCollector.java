
package org.elasticsearch.xpack.ml.job.persistence.overallbuckets;

import org.elasticsearch.xpack.core.ml.job.results.OverallBucket;

import java.util.ArrayList;
import java.util.List;

public class OverallBucketsCollector implements OverallBucketsProcessor {

    private final List<OverallBucket> collected = new ArrayList<>();

    @Override
    public synchronized void process(List<OverallBucket> overallBuckets) {
        collected.addAll(overallBuckets);
    }

    @Override
    public synchronized List<OverallBucket> finish() {
        return collected;
    }

    @Override
    public synchronized int size() {
        return collected.size();
    }
}
