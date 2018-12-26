

package org.elasticsearch.painless.node;

import org.elasticsearch.painless.DefBootstrap;

import org.elasticsearch.painless.Definition.def;
import org.elasticsearch.painless.Globals;
import org.elasticsearch.painless.Locals;
import org.elasticsearch.painless.Location;
import org.elasticsearch.painless.MethodWriter;
import org.objectweb.asm.Type;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.Set;


final class PSubDefCall extends AExpression {

    private final String name;
    private final List<AExpression> arguments;

    private StringBuilder recipe = null;
    private List<String> pointers = new ArrayList<>();

    PSubDefCall(Location location, String name, List<AExpression> arguments) {
        super(location);

        this.name = Objects.requireNonNull(name);
        this.arguments = Objects.requireNonNull(arguments);
    }

    @Override
    void extractVariables(Set<String> variables) {
        throw createError(new IllegalStateException("Illegal tree structure."));
    }

    @Override
    void analyze(Locals locals) {
        recipe = new StringBuilder();
        int totalCaptures = 0;

        for (int argument = 0; argument < arguments.size(); ++argument) {
            AExpression expression = arguments.get(argument);

            expression.internal = true;
            expression.analyze(locals);

            if (expression instanceof ILambda) {
                ILambda lambda = (ILambda) expression;
                pointers.add(lambda.getPointer());
                                char ch = (char) (argument + totalCaptures);
                recipe.append(ch);
                totalCaptures += lambda.getCaptureCount();
            }

            if (expression.actual == void.class) {
                throw createError(new IllegalArgumentException("Argument(s) cannot be of [void] type when calling method [" + name + "]."));
            }

            expression.expected = expression.actual;
            arguments.set(argument, expression.cast(locals));
        }

        actual = expected == null || explicit ? def.class : expected;
    }

    @Override
    void write(MethodWriter writer, Globals globals) {
        writer.writeDebugInfo(location);

        List<Type> parameterTypes = new ArrayList<>();

                parameterTypes.add(org.objectweb.asm.Type.getType(Object.class));

                for (AExpression argument : arguments) {
            parameterTypes.add(MethodWriter.getType(argument.actual));

            if (argument instanceof ILambda) {
                ILambda lambda = (ILambda) argument;
                Collections.addAll(parameterTypes, lambda.getCaptures());
            }

            argument.write(writer, globals);
        }

                Type methodType = Type.getMethodType(MethodWriter.getType(actual), parameterTypes.toArray(new Type[0]));

        List<Object> args = new ArrayList<>();
        args.add(recipe.toString());
        args.addAll(pointers);
        writer.invokeDefCall(name, methodType, DefBootstrap.METHOD_CALL, args.toArray());
    }

    @Override
    public String toString() {
        return singleLineToStringWithOptionalArgs(arguments, prefix, name);
    }
}
