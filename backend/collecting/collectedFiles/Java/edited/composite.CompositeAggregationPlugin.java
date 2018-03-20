

package org.elasticsearch.search.aggregations.bucket.composite;

import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.plugins.SearchPlugin;

import java.util.Arrays;
import java.util.List;

public class CompositeAggregationPlugin extends Plugin implements SearchPlugin {
    @Override
    public List<AggregationSpec> getAggregations() {
        return Arrays.asList(
            new AggregationSpec(CompositeAggregationBuilder.NAME, CompositeAggregationBuilder::new, CompositeAggregationBuilder::parse)
                .addResultReader(InternalComposite::new)
        );
    }
}
