

package org.elasticsearch.search.aggregations.matrix;

import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.plugins.SearchPlugin;
import org.elasticsearch.search.aggregations.matrix.stats.InternalMatrixStats;
import org.elasticsearch.search.aggregations.matrix.stats.MatrixStatsAggregationBuilder;
import org.elasticsearch.search.aggregations.matrix.stats.MatrixStatsParser;

import java.util.List;

import static java.util.Collections.singletonList;

public class MatrixAggregationPlugin extends Plugin implements SearchPlugin {
    @Override
    public List<AggregationSpec> getAggregations() {
        return singletonList(new AggregationSpec(MatrixStatsAggregationBuilder.NAME, MatrixStatsAggregationBuilder::new,
                new MatrixStatsParser()).addResultReader(InternalMatrixStats::new));
    }
}
