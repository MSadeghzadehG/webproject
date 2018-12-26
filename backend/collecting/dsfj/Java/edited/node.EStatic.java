

package org.elasticsearch.painless.node;

import org.elasticsearch.painless.Definition;
import org.elasticsearch.painless.Globals;
import org.elasticsearch.painless.Locals;
import org.elasticsearch.painless.Location;
import org.elasticsearch.painless.MethodWriter;

import java.util.Objects;
import java.util.Set;


public final class EStatic extends AExpression {

    private final String type;

    public EStatic(Location location, String type) {
        super(location);

        this.type = Objects.requireNonNull(type);
    }

    @Override
    void extractVariables(Set<String> variables) {
            }

    @Override
    void analyze(Locals locals) {
        try {
            actual = Definition.TypeToClass(locals.getDefinition().getType(type));
        } catch (IllegalArgumentException exception) {
            throw createError(new IllegalArgumentException("Not a type [" + type + "]."));
        }
    }

    @Override
    void write(MethodWriter writer, Globals globals) {
            }

    @Override
    public String toString() {
        return singleLineToString(type);
    }
}
