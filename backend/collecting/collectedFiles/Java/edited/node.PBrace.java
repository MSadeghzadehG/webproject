

package org.elasticsearch.painless.node;

import org.elasticsearch.painless.Definition;
import org.elasticsearch.painless.Definition.def;
import org.elasticsearch.painless.Globals;
import org.elasticsearch.painless.Locals;
import org.elasticsearch.painless.Location;
import org.elasticsearch.painless.MethodWriter;

import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;


public final class PBrace extends AStoreable {

    private AExpression index;

    private AStoreable sub = null;

    public PBrace(Location location, AExpression prefix, AExpression index) {
        super(location, prefix);

        this.index = Objects.requireNonNull(index);
    }

    @Override
    void extractVariables(Set<String> variables) {
        prefix.extractVariables(variables);
        index.extractVariables(variables);
    }

    @Override
    void analyze(Locals locals) {
        prefix.analyze(locals);
        prefix.expected = prefix.actual;
        prefix = prefix.cast(locals);

        if (prefix.actual.isArray()) {
            sub = new PSubBrace(location, prefix.actual, index);
        } else if (prefix.actual == def.class) {
            sub = new PSubDefArray(location, index);
        } else if (Map.class.isAssignableFrom(prefix.actual)) {
            sub = new PSubMapShortcut(location, locals.getDefinition().ClassToType(prefix.actual).struct, index);
        } else if (List.class.isAssignableFrom(prefix.actual)) {
            sub = new PSubListShortcut(location, locals.getDefinition().ClassToType(prefix.actual).struct, index);
        } else {
            throw createError(
                new IllegalArgumentException("Illegal array access on type [" + Definition.ClassToName(prefix.actual) + "]."));
        }

        sub.write = write;
        sub.read = read;
        sub.expected = expected;
        sub.explicit = explicit;
        sub.analyze(locals);
        actual = sub.actual;
    }

    @Override
    void write(MethodWriter writer, Globals globals) {
        prefix.write(writer, globals);
        sub.write(writer, globals);
    }

    @Override
    boolean isDefOptimized() {
        return sub.isDefOptimized();
    }

    @Override
    void updateActual(Class<?> actual) {
        sub.updateActual(actual);
        this.actual = actual;
    }

    @Override
    int accessElementCount() {
        return sub.accessElementCount();
    }

    @Override
    void setup(MethodWriter writer, Globals globals) {
        prefix.write(writer, globals);
        sub.setup(writer, globals);
    }

    @Override
    void load(MethodWriter writer, Globals globals) {
        sub.load(writer, globals);
    }

    @Override
    void store(MethodWriter writer, Globals globals) {
        sub.store(writer, globals);
    }

    @Override
    public String toString() {
        return singleLineToString(prefix, index);
    }
}
