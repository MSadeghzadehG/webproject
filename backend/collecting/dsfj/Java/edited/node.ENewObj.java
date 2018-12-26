

package org.elasticsearch.painless.node;

import org.elasticsearch.painless.Definition;
import org.elasticsearch.painless.Definition.Method;
import org.elasticsearch.painless.Definition.Struct;
import org.elasticsearch.painless.Definition.Type;
import org.elasticsearch.painless.Globals;
import org.elasticsearch.painless.Locals;
import org.elasticsearch.painless.Location;
import org.elasticsearch.painless.MethodWriter;

import java.util.List;
import java.util.Objects;
import java.util.Set;


public final class ENewObj extends AExpression {

    private final String type;
    private final List<AExpression> arguments;

    private Method constructor;

    public ENewObj(Location location, String type, List<AExpression> arguments) {
        super(location);

        this.type = Objects.requireNonNull(type);
        this.arguments = Objects.requireNonNull(arguments);
    }

    @Override
    void extractVariables(Set<String> variables) {
        for (AExpression argument : arguments) {
            argument.extractVariables(variables);
        }
    }

    @Override
    void analyze(Locals locals) {
        try {
            actual = Definition.TypeToClass(locals.getDefinition().getType(this.type));
        } catch (IllegalArgumentException exception) {
            throw createError(new IllegalArgumentException("Not a type [" + this.type + "]."));
        }

        Struct struct = locals.getDefinition().ClassToType(actual).struct;
        constructor = struct.constructors.get(new Definition.MethodKey("<init>", arguments.size()));

        if (constructor != null) {
            Class<?>[] types = new Class<?>[constructor.arguments.size()];
            constructor.arguments.toArray(types);

            if (constructor.arguments.size() != arguments.size()) {
                throw createError(new IllegalArgumentException("When calling constructor on type [" + struct.name + "]" +
                    " expected [" + constructor.arguments.size() + "] arguments, but found [" + arguments.size() + "]."));
            }

            for (int argument = 0; argument < arguments.size(); ++argument) {
                AExpression expression = arguments.get(argument);

                expression.expected = types[argument];
                expression.internal = true;
                expression.analyze(locals);
                arguments.set(argument, expression.cast(locals));
            }

            statement = true;
        } else {
            throw createError(new IllegalArgumentException("Unknown new call on type [" + struct.name + "]."));
        }
    }

    @Override
    void write(MethodWriter writer, Globals globals) {
        writer.writeDebugInfo(location);

        writer.newInstance(MethodWriter.getType(actual));

        if (read) {
            writer.dup();
        }

        for (AExpression argument : arguments) {
            argument.write(writer, globals);
        }

        writer.invokeConstructor(constructor.owner.type, constructor.method);
    }

    @Override
    public String toString() {
        return singleLineToStringWithOptionalArgs(arguments, type);
    }
}
