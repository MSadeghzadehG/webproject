

package org.elasticsearch.painless.node;

import org.elasticsearch.painless.Definition;
import org.elasticsearch.painless.Definition.Field;
import org.elasticsearch.painless.Globals;
import org.elasticsearch.painless.Locals;
import org.elasticsearch.painless.Location;
import org.elasticsearch.painless.MethodWriter;

import java.lang.reflect.Modifier;
import java.util.Objects;
import java.util.Set;


final class PSubField extends AStoreable {

    private final Field field;

    PSubField(Location location, Field field) {
        super(location);

        this.field = Objects.requireNonNull(field);
    }

    @Override
    void extractVariables(Set<String> variables) {
        throw createError(new IllegalStateException("Illegal tree structure."));
    }

    @Override
    void analyze(Locals locals) {
         if (write && Modifier.isFinal(field.modifiers)) {
             throw createError(new IllegalArgumentException(
                 "Cannot write to read-only field [" + field.name + "] for type [" + Definition.ClassToName(field.clazz) + "]."));
         }

        actual = field.clazz;
    }

    @Override
    void write(MethodWriter writer, Globals globals) {
        writer.writeDebugInfo(location);

        if (java.lang.reflect.Modifier.isStatic(field.modifiers)) {
            writer.getStatic(field.owner.type, field.javaName, MethodWriter.getType(field.clazz));
        } else {
            writer.getField(field.owner.type, field.javaName, MethodWriter.getType(field.clazz));
        }
    }

    @Override
    int accessElementCount() {
        return 1;
    }

    @Override
    boolean isDefOptimized() {
        return false;
    }

    @Override
    void updateActual(Class<?> actual) {
        throw new IllegalArgumentException("Illegal tree structure.");
    }

    @Override
    void setup(MethodWriter writer, Globals globals) {
            }

    @Override
    void load(MethodWriter writer, Globals globals) {
        writer.writeDebugInfo(location);

        if (java.lang.reflect.Modifier.isStatic(field.modifiers)) {
            writer.getStatic(field.owner.type, field.javaName, MethodWriter.getType(field.clazz));
        } else {
            writer.getField(field.owner.type, field.javaName, MethodWriter.getType(field.clazz));
        }
    }

    @Override
    void store(MethodWriter writer, Globals globals) {
        writer.writeDebugInfo(location);

        if (java.lang.reflect.Modifier.isStatic(field.modifiers)) {
            writer.putStatic(field.owner.type, field.javaName, MethodWriter.getType(field.clazz));
        } else {
            writer.putField(field.owner.type, field.javaName, MethodWriter.getType(field.clazz));
        }
    }

    @Override
    public String toString() {
        return singleLineToString(prefix, field.name);
    }
}
