

package org.elasticsearch.common.inject.name;

import java.lang.annotation.Annotation;
import java.util.Objects;

class NamedImpl implements Named {

    private final String value;

    NamedImpl(String value) {
        this.value = Objects.requireNonNull(value, "name");
    }

    @Override
    public String value() {
        return this.value;
    }

    @Override
    public int hashCode() {
                return (127 * "value".hashCode()) ^ value.hashCode();
    }

    @Override
    public boolean equals(Object o) {
        if (!(o instanceof Named)) {
            return false;
        }

        Named other = (Named) o;
        return value.equals(other.value());
    }

    @Override
    public String toString() {
        return "@" + Named.class.getName() + "(value=" + value + ")";
    }

    @Override
    public Class<? extends Annotation> annotationType() {
        return Named.class;
    }
}
