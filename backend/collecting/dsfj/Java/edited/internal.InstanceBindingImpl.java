

package org.elasticsearch.common.inject.internal;

import org.elasticsearch.common.inject.Binder;
import org.elasticsearch.common.inject.Injector;
import org.elasticsearch.common.inject.Key;
import org.elasticsearch.common.inject.Provider;
import org.elasticsearch.common.inject.spi.BindingTargetVisitor;
import org.elasticsearch.common.inject.spi.Dependency;
import org.elasticsearch.common.inject.spi.HasDependencies;
import org.elasticsearch.common.inject.spi.InjectionPoint;
import org.elasticsearch.common.inject.spi.InstanceBinding;
import org.elasticsearch.common.inject.util.Providers;

import java.util.HashSet;
import java.util.Set;

import static java.util.Collections.unmodifiableSet;

public class InstanceBindingImpl<T> extends BindingImpl<T> implements InstanceBinding<T> {

    final T instance;
    final Provider<T> provider;
    final Set<InjectionPoint> injectionPoints;

    public InstanceBindingImpl(Injector injector, Key<T> key, Object source,
                               InternalFactory<? extends T> internalFactory, Set<InjectionPoint> injectionPoints,
                               T instance) {
        super(injector, key, source, internalFactory, Scoping.UNSCOPED);
        this.injectionPoints = injectionPoints;
        this.instance = instance;
        this.provider = Providers.of(instance);
    }

    public InstanceBindingImpl(Object source, Key<T> key, Scoping scoping,
                               Set<InjectionPoint> injectionPoints, T instance) {
        super(source, key, scoping);
        this.injectionPoints = injectionPoints;
        this.instance = instance;
        this.provider = Providers.of(instance);
    }

    @Override
    public Provider<T> getProvider() {
        return this.provider;
    }

    @Override
    public <V> V acceptTargetVisitor(BindingTargetVisitor<? super T, V> visitor) {
        return visitor.visit(this);
    }

    @Override
    public T getInstance() {
        return instance;
    }

    @Override
    public Set<InjectionPoint> getInjectionPoints() {
        return injectionPoints;
    }

    @Override
    public Set<Dependency<?>> getDependencies() {
        return instance instanceof HasDependencies
                ? unmodifiableSet(new HashSet<>((((HasDependencies) instance).getDependencies())))
                : Dependency.forInjectionPoints(injectionPoints);
    }

    @Override
    public BindingImpl<T> withScoping(Scoping scoping) {
        return new InstanceBindingImpl<>(getSource(), getKey(), scoping, injectionPoints, instance);
    }

    @Override
    public BindingImpl<T> withKey(Key<T> key) {
        return new InstanceBindingImpl<>(getSource(), key, getScoping(), injectionPoints, instance);
    }

    @Override
    public void applyTo(Binder binder) {
                binder.withSource(getSource()).bind(getKey()).toInstance(instance);
    }

    @Override
    public String toString() {
        return new ToStringBuilder(InstanceBinding.class)
                .add("key", getKey())
                .add("source", getSource())
                .add("instance", instance)
                .toString();
    }
}
