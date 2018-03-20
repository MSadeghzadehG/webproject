
package org.elasticsearch.search.aggregations;

import org.apache.lucene.util.SetOnce;
import org.elasticsearch.common.ParsingException;
import org.elasticsearch.common.xcontent.ToXContentFragment;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.common.xcontent.XContentParser;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;

import static java.util.Collections.unmodifiableMap;
import static org.elasticsearch.common.xcontent.XContentParserUtils.parseTypedKeysObject;


public class Aggregations implements Iterable<Aggregation>, ToXContentFragment {

    public static final String AGGREGATIONS_FIELD = "aggregations";

    protected List<? extends Aggregation> aggregations = Collections.emptyList();
    protected Map<String, Aggregation> aggregationsAsMap;

    protected Aggregations() {
    }

    public Aggregations(List<? extends Aggregation> aggregations) {
        this.aggregations = aggregations;
    }

    
    @Override
    public final Iterator<Aggregation> iterator() {
        return aggregations.stream().map((p) -> (Aggregation) p).iterator();
    }

    
    public final List<Aggregation> asList() {
        return Collections.unmodifiableList(aggregations);
    }

    
    public final Map<String, Aggregation> asMap() {
        return getAsMap();
    }

    
    public final Map<String, Aggregation> getAsMap() {
        if (aggregationsAsMap == null) {
            Map<String, Aggregation> newAggregationsAsMap = new HashMap<>(aggregations.size());
            for (Aggregation aggregation : aggregations) {
                newAggregationsAsMap.put(aggregation.getName(), aggregation);
            }
            this.aggregationsAsMap = unmodifiableMap(newAggregationsAsMap);
        }
        return aggregationsAsMap;
    }

    
    @SuppressWarnings("unchecked")
    public final <A extends Aggregation> A get(String name) {
        return (A) asMap().get(name);
    }

    @Override
    public final boolean equals(Object obj) {
        if (obj == null || getClass() != obj.getClass()) {
            return false;
        }
        return aggregations.equals(((Aggregations) obj).aggregations);
    }

    @Override
    public final int hashCode() {
        return Objects.hash(getClass(), aggregations);
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        if (aggregations.isEmpty()) {
            return builder;
        }
        builder.startObject(AGGREGATIONS_FIELD);
        toXContentInternal(builder, params);
        return builder.endObject();
    }

    
    public XContentBuilder toXContentInternal(XContentBuilder builder, Params params) throws IOException {
        for (Aggregation aggregation : aggregations) {
            aggregation.toXContent(builder, params);
        }
        return builder;
    }

    public static Aggregations fromXContent(XContentParser parser) throws IOException {
        final List<Aggregation> aggregations = new ArrayList<>();
        XContentParser.Token token;
        while ((token = parser.nextToken()) != XContentParser.Token.END_OBJECT) {
            if (token == XContentParser.Token.START_OBJECT) {
                SetOnce<Aggregation> typedAgg = new SetOnce<>();
                String currentField = parser.currentName();
                parseTypedKeysObject(parser, Aggregation.TYPED_KEYS_DELIMITER, Aggregation.class, typedAgg::set);
                if (typedAgg.get() != null) {
                    aggregations.add(typedAgg.get());
                } else {
                    throw new ParsingException(parser.getTokenLocation(),
                            String.format(Locale.ROOT, "Could not parse aggregation keyed as [%s]", currentField));
                }
            }
        }
        return new Aggregations(aggregations);
    }
}
