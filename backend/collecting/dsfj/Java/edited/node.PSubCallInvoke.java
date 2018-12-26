

package org.elasticsearch.painless.node;

import org.elasticsearch.painless.Definition;
import org.elasticsearch.painless.Definition.Method;
import org.elasticsearch.painless.Globals;
import org.elasticsearch.painless.Locals;
import org.elasticsearch.painless.Location;
import org.elasticsearch.painless.MethodWriter;

import java.util.List;
import java.util.Objects;
import java.util.Set;


final class PSubCallInvoke extends AExpression {

    private final Method method;
    private final Class<?> box;
    private final List<AExpression> arguments;

    PSubCallInvoke(Location location, Method method, Class<?> box, List<AExpression> arguments) {
        super(location);

        this.method = Objects.requireNonNull(method);
        this.box = box;
        this.arguments = Objects.requireNonNull(arguments);
    }

    @Override
    void extractVariables(Set<String> variables) {
        throw createError(new IllegalStateException("Illegal tree structure."));
    }

    @Override
    void analyze(Locals locals) {
        for (int argument = 0; argument < arguments.size(); ++argument) {
            AExpression expression = arguments.get(argument);

            expression.expected = method.arguments.get(argument);
            expression.internal = true;
            expression.analyze(locals);
            arguments.set(argument, expression.cast(locals));
        }

        statement = true;
        actual = method.rtn;
    }

    @Override
    void write(MethodWriter writer, Globals globals) {
        writer.writeDebugInfo(location);

        if (box.isPrimitive()) {
            writer.box(MethodWriter.getType(box));
        }

        for (AExpression argument : arguments) {
            argument.write(writer, globals);
        }

        method.write(writer);
    }

    @Override
    public String toString() {
        return singleLineToStringWithOptionalArgs(arguments, prefix, method.name);
    }
}
