

package org.elasticsearch.painless.node;

import org.elasticsearch.painless.Definition;
import org.elasticsearch.painless.Globals;
import org.elasticsearch.painless.Locals;
import org.elasticsearch.painless.Location;
import org.elasticsearch.painless.MethodWriter;

import java.util.Objects;
import java.util.Set;


public final class SReturn extends AStatement {

    private AExpression expression;

    public SReturn(Location location, AExpression expression) {
        super(location);

        this.expression = Objects.requireNonNull(expression);
    }

    @Override
    void extractVariables(Set<String> variables) {
        expression.extractVariables(variables);
    }

    @Override
    void analyze(Locals locals) {
        expression.expected = locals.getReturnType();
        expression.internal = true;
        expression.analyze(locals);
        expression = expression.cast(locals);

        methodEscape = true;
        loopEscape = true;
        allEscape = true;

        statementCount = 1;
    }

    @Override
    void write(MethodWriter writer, Globals globals) {
        writer.writeStatementOffset(location);
        expression.write(writer, globals);
        writer.returnValue();
    }

    @Override
    public String toString() {
        return singleLineToString(expression);
    }
}
