

package org.elasticsearch.join.aggregations;

import org.elasticsearch.common.ParseField;
import org.elasticsearch.common.io.stream.Writeable.Reader;
import org.elasticsearch.common.xcontent.NamedXContentRegistry;
import org.elasticsearch.common.xcontent.NamedXContentRegistry.Entry;
import org.elasticsearch.search.aggregations.Aggregation;
import org.elasticsearch.search.aggregations.InternalAggregations;
import org.elasticsearch.search.aggregations.InternalSingleBucketAggregationTestCase;
import org.elasticsearch.search.aggregations.bucket.ParsedSingleBucketAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class InternalChildrenTests extends InternalSingleBucketAggregationTestCase<InternalChildren> {

    @Override
    protected List<NamedXContentRegistry.Entry> getNamedXContents() {
        List<Entry> extendedNamedXContents = new ArrayList<>(super.getNamedXContents());
        extendedNamedXContents.add(new NamedXContentRegistry.Entry(Aggregation.class, new ParseField(ChildrenAggregationBuilder.NAME),
                (p, c) -> ParsedChildren.fromXContent(p, (String) c)));
        return extendedNamedXContents ;
    }

    @Override
    protected InternalChildren createTestInstance(String name, long docCount, InternalAggregations aggregations,
            List<PipelineAggregator> pipelineAggregators, Map<String, Object> metaData) {
        return new InternalChildren(name, docCount, aggregations, pipelineAggregators, metaData);
    }

    @Override
    protected void extraAssertReduced(InternalChildren reduced, List<InternalChildren> inputs) {
            }

    @Override
    protected Reader<InternalChildren> instanceReader() {
        return InternalChildren::new;
    }

    @Override
    protected Class<? extends ParsedSingleBucketAggregation> implementationClass() {
        return ParsedChildren.class;
    }
}
