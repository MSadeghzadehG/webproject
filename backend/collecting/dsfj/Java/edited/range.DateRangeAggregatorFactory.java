

package org.elasticsearch.search.aggregations.bucket.range;

import org.elasticsearch.search.aggregations.AggregatorFactories;
import org.elasticsearch.search.aggregations.AggregatorFactory;
import org.elasticsearch.search.aggregations.support.ValuesSource.Numeric;
import org.elasticsearch.search.aggregations.support.ValuesSourceConfig;
import org.elasticsearch.search.internal.SearchContext;

import java.io.IOException;
import java.util.Map;

public class DateRangeAggregatorFactory extends AbstractRangeAggregatorFactory<DateRangeAggregatorFactory, RangeAggregator.Range> {

    public DateRangeAggregatorFactory(String name, ValuesSourceConfig<Numeric> config, RangeAggregator.Range[] ranges, boolean keyed,
            InternalRange.Factory<?, ?> rangeFactory, SearchContext context, AggregatorFactory<?> parent,
            AggregatorFactories.Builder subFactoriesBuilder, Map<String, Object> metaData) throws IOException {
        super(name, config, ranges, keyed, rangeFactory, context, parent, subFactoriesBuilder, metaData);
    }

}
