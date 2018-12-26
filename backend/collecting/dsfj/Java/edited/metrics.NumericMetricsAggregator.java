
package org.elasticsearch.search.aggregations.metrics;

import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public abstract class NumericMetricsAggregator extends MetricsAggregator {

    private NumericMetricsAggregator(String name, SearchContext context, Aggregator parent,
            List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData) throws IOException {
        super(name, context, parent, pipelineAggregators, metaData);
    }

    public abstract static class SingleValue extends NumericMetricsAggregator {

        protected SingleValue(String name, SearchContext context, Aggregator parent, List<PipelineAggregator> pipelineAggregators,
                Map<String, Object> metaData) throws IOException {
            super(name, context, parent, pipelineAggregators, metaData);
        }

        public abstract double metric(long owningBucketOrd);
    }

    public abstract static class MultiValue extends NumericMetricsAggregator {

        protected MultiValue(String name, SearchContext context, Aggregator parent, List<PipelineAggregator> pipelineAggregators,
                Map<String, Object> metaData) throws IOException {
            super(name, context, parent, pipelineAggregators, metaData);
        }

        public abstract boolean hasMetric(String name);

        public abstract double metric(String name, long owningBucketOrd);
    }
}
