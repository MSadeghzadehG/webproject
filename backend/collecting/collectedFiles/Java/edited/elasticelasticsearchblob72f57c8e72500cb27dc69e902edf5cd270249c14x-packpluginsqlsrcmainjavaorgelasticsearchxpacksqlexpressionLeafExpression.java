
package org.elasticsearch.xpack.sql.expression;

import org.elasticsearch.xpack.sql.tree.Location;

import java.util.List;

import static java.util.Collections.emptyList;

public abstract class LeafExpression extends Expression {

    protected LeafExpression(Location location) {
        super(location, emptyList());
    }

    @Override
    public final Expression replaceChildren(List<Expression> newChildren) {
        throw new UnsupportedOperationException("this type of node doesn't have any children to replace");
    }

    public AttributeSet references() {
        return AttributeSet.EMPTY;
    }
}
