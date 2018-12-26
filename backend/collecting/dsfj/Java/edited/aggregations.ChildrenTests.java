

package org.elasticsearch.join.aggregations;

import org.elasticsearch.join.ParentJoinPlugin;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.search.aggregations.BaseAggregationTestCase;

import java.util.Collection;
import java.util.Collections;

public class ChildrenTests extends BaseAggregationTestCase<ChildrenAggregationBuilder> {

    @Override
    protected Collection<Class<? extends Plugin>> getPlugins() {
        return Collections.singleton(ParentJoinPlugin.class);
    }

    @Override
    protected ChildrenAggregationBuilder createTestAggregatorBuilder() {
        String name = randomAlphaOfLengthBetween(3, 20);
        String childType = randomAlphaOfLengthBetween(5, 40);
        ChildrenAggregationBuilder factory = new ChildrenAggregationBuilder(name, childType);
        return factory;
    }

}
