

package org.elasticsearch.search.profile.aggregation;

import org.elasticsearch.search.aggregations.Aggregator;
import org.elasticsearch.search.profile.AbstractProfiler;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class AggregationProfiler extends AbstractProfiler<AggregationProfileBreakdown, Aggregator> {

    private final Map<List<String>, AggregationProfileBreakdown> profileBrakdownLookup = new HashMap<>();

    public AggregationProfiler() {
        super(new InternalAggregationProfileTree());
    }

    @Override
    public AggregationProfileBreakdown getQueryBreakdown(Aggregator agg) {
        List<String> path = getAggregatorPath(agg);
        AggregationProfileBreakdown aggregationProfileBreakdown = profileBrakdownLookup.get(path);
        if (aggregationProfileBreakdown == null) {
            aggregationProfileBreakdown = super.getQueryBreakdown(agg);
            profileBrakdownLookup.put(path, aggregationProfileBreakdown);
        }
        return aggregationProfileBreakdown;
    }

    public static List<String> getAggregatorPath(Aggregator agg) {
        LinkedList<String> path = new LinkedList<>();
        while (agg != null) {
            path.addFirst(agg.name());
            agg = agg.parent();
        }
        return path;
    }
}
