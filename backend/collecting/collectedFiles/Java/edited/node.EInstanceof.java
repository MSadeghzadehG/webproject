

package org.elasticsearch.painless.node;

import org.elasticsearch.painless.Definition;
import org.elasticsearch.painless.Globals;
import org.elasticsearch.painless.Locals;
import org.elasticsearch.painless.Location;
import org.elasticsearch.painless.MethodWriter;

import java.util.Objects;
import java.util.Set;


public final class EInstanceof extends AExpression {
    private AExpression expression;
    private final String type;

    private Class<?> resolvedType;
    private Class<?> expressionType;
    private boolean primitiveExpression;

    public EInstanceof(Location location, AExpression expression, String type) {
        super(location);
        this.expression = Objects.requireNonNull(expression);
        this.type = Objects.requireNonNull(type);
    }

    @Override
    void extractVariables(Set<String> variables) {
        expression.extractVariables(variables);
    }

    @Override
    void analyze(Locals locals) {
        Class<?> clazz;

                try {
            clazz = Definition.TypeToClass(locals.getDefinition().getType(this.type));
        } catch (IllegalArgumentException exception) {
            throw createError(new IllegalArgumentException("Not a type [" + this.type + "]."));
        }

                resolvedType = clazz.isPrimitive() ? Definition.getBoxedType(clazz) : Definition.defClassToObjectClass(clazz);

                expression.analyze(locals);
        expression.expected = expression.actual;
        expression = expression.cast(locals);

                primitiveExpression = expression.actual.isPrimitive();
                expressionType = expression.actual.isPrimitive() ?
            Definition.getBoxedType(expression.actual) : Definition.defClassToObjectClass(clazz);

        actual = boolean.class;
    }

    @Override
    void write(MethodWriter writer, Globals globals) {
                if (primitiveExpression) {
                        expression.write(writer, globals);
                        writer.writePop(MethodWriter.getType(expression.actual).getSize());
                        writer.push(resolvedType.isAssignableFrom(expressionType));
        } else {
                        expression.write(writer, globals);
            writer.instanceOf(org.objectweb.asm.Type.getType(resolvedType));
        }
    }

    @Override
    public String toString() {
        return singleLineToString(expression, type);
    }
}
