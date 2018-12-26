

package org.elasticsearch.search.aggregations.metrics.geobounds;

import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.AggregatorFactory;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.search.aggregations.support.ValuesSource;
import org.elasticsearch.search.aggregations.support.ValuesSourceAggregatorFactory;
import org.elasticsearch.search.aggregations.support.ValuesSourceConfig;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class GeoBoundsAggregatorFactory extends ValuesSourceAggregatorFactory<ValuesSource.GeoPoint, GeoBoundsAggregatorFactory> {

    private final boolean wrapLongitude;

    public GeoBoundsAggregatorFactory(String name, ValuesSourceConfig<ValuesSource.GeoPoint> config, boolean wrapLongitude,
            SearchContext context, AggregatorFactory<?> parent, AggregatorFactories.Builder subFactoriesBuilder,
            Map<String, Object> metaData) throws IOException {
        super(name, config, context, parent, subFactoriesBuilder, metaData);
        this.wrapLongitude = wrapLongitude;
    }

    @Override
    protected Aggregator createUnmapped(Aggregator parent, List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData)
            throws IOException {
        return new GeoBoundsAggregator(name, context, parent, null, wrapLongitude, pipelineAggregators, metaData);
    }

    @Override
    protected Aggregator doCreateInternal(ValuesSource.GeoPoint valuesSource, Aggregator parent, boolean collectsFromSingleBucket,
            List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData) throws IOException {
        return new GeoBoundsAggregator(name, context, parent, valuesSource, wrapLongitude, pipelineAggregators, metaData);
    }
}
