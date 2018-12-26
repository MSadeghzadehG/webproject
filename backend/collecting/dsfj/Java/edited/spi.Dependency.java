

package org.elasticsearch.common.inject.spi;

import org.elasticsearch.common.inject.Key;

import java.util.HashSet;
import java.util.Objects;
import java.util.Set;

import static java.util.Collections.unmodifiableSet;


public final class Dependency<T> {
    private final InjectionPoint injectionPoint;
    private final Key<T> key;
    private final boolean nullable;
    private final int parameterIndex;

    Dependency(InjectionPoint injectionPoint, Key<T> key,
               boolean nullable, int parameterIndex) {
        this.injectionPoint = injectionPoint;
        this.key = key;
        this.nullable = nullable;
        this.parameterIndex = parameterIndex;
    }

    
    public static <T> Dependency<T> get(Key<T> key) {
        return new Dependency<>(null, key, true, -1);
    }

    
    public static Set<Dependency<?>> forInjectionPoints(Set<InjectionPoint> injectionPoints) {
        Set<Dependency<?>> dependencies = new HashSet<>();
        for (InjectionPoint injectionPoint : injectionPoints) {
            dependencies.addAll(injectionPoint.getDependencies());
        }
        return unmodifiableSet(dependencies);
    }

    
    public Key<T> getKey() {
        return this.key;
    }

    
    public boolean isNullable() {
        return nullable;
    }

    
    public InjectionPoint getInjectionPoint() {
        return injectionPoint;
    }

    
    public int getParameterIndex() {
        return parameterIndex;
    }

    @Override
    public int hashCode() {
        return Objects.hash(injectionPoint, parameterIndex, key);
    }

    @Override
    public boolean equals(Object o) {
        if (o instanceof Dependency) {
            Dependency dependency = (Dependency) o;
            return Objects.equals(injectionPoint, dependency.injectionPoint)
                    && Objects.equals(parameterIndex, dependency.parameterIndex)
                    && Objects.equals(key, dependency.key);
        } else {
            return false;
        }
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append(key);
        if (injectionPoint != null) {
            builder.append("@").append(injectionPoint);
            if (parameterIndex != -1) {
                builder.append("[").append(parameterIndex).append("]");
            }
        }
        return builder.toString();
    }
}
