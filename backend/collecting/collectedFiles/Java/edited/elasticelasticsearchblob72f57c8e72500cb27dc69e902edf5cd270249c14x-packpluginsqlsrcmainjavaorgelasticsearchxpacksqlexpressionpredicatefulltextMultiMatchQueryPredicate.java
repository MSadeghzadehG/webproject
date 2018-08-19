
package org.elasticsearch.xpack.sql.expression.predicate.fulltext;

import java.util.Map;
import java.util.Objects;

import org.elasticsearch.xpack.sql.expression.Expression;
import org.elasticsearch.xpack.sql.tree.Location;
import org.elasticsearch.xpack.sql.tree.NodeInfo;

import static java.util.Collections.emptyList;

import java.util.List;

public class MultiMatchQueryPredicate extends FullTextPredicate {

    private final String fieldString;
    private final Map<String, Float> fields;

    public MultiMatchQueryPredicate(Location location, String fieldString, String query, String options) {
        super(location, query, options, emptyList());
        this.fieldString = fieldString;
                this.fields = FullTextUtils.parseFields(fieldString, location);
    }

    @Override
    protected NodeInfo<MultiMatchQueryPredicate> info() {
        return NodeInfo.create(this, MultiMatchQueryPredicate::new, fieldString, query(), options());
    }

    @Override
    public Expression replaceChildren(List<Expression> newChildren) {
        throw new UnsupportedOperationException("this type of node doesn't have any children to replace");
    }

    public String fieldString() {
        return fieldString;
    }

    public Map<String, Float> fields() {
        return fields;
    }

    @Override
    public int hashCode() {
        return Objects.hash(fieldString, super.hashCode());
    }

    @Override
    public boolean equals(Object obj) {
        if (super.equals(obj)) {
            MultiMatchQueryPredicate other = (MultiMatchQueryPredicate) obj;
            return Objects.equals(fieldString, other.fieldString);
        }
        return false;
    }
}
