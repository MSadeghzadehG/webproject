

package org.elasticsearch.common.inject;

import org.elasticsearch.common.inject.internal.BindingImpl;
import org.elasticsearch.common.inject.internal.Errors;
import org.elasticsearch.common.inject.internal.MatcherAndConverter;
import org.elasticsearch.common.inject.spi.TypeListenerBinding;

import java.lang.annotation.Annotation;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import static java.util.Collections.emptySet;


interface State {

    State NONE = new State() {
        @Override
        public State parent() {
            throw new UnsupportedOperationException();
        }

        @Override
        public <T> BindingImpl<T> getExplicitBinding(Key<T> key) {
            return null;
        }

        @Override
        public Map<Key<?>, Binding<?>> getExplicitBindingsThisLevel() {
            throw new UnsupportedOperationException();
        }

        @Override
        public void putBinding(Key<?> key, BindingImpl<?> binding) {
            throw new UnsupportedOperationException();
        }

        @Override
        public Scope getScope(Class<? extends Annotation> scopingAnnotation) {
            return null;
        }

        @Override
        public void putAnnotation(Class<? extends Annotation> annotationType, Scope scope) {
            throw new UnsupportedOperationException();
        }

        @Override
        public void addConverter(MatcherAndConverter matcherAndConverter) {
            throw new UnsupportedOperationException();
        }

        @Override
        public MatcherAndConverter getConverter(String stringValue, TypeLiteral<?> type, Errors errors,
                                                Object source) {
            throw new UnsupportedOperationException();
        }

        @Override
        public Iterable<MatcherAndConverter> getConvertersThisLevel() {
            return emptySet();
        }

        @Override
        public void addTypeListener(TypeListenerBinding typeListenerBinding) {
            throw new UnsupportedOperationException();
        }

        @Override
        public List<TypeListenerBinding> getTypeListenerBindings() {
            return Collections.emptyList();
        }

        @Override
        public void blacklist(Key<?> key) {
        }

        @Override
        public boolean isBlacklisted(Key<?> key) {
            return true;
        }

        @Override
        public void clearBlacklisted() {
        }

        @Override
        public void makeAllBindingsToEagerSingletons(Injector injector) {
        }

        @Override
        public Object lock() {
            throw new UnsupportedOperationException();
        }
    };

    State parent();

    
    <T> BindingImpl<T> getExplicitBinding(Key<T> key);

    
    Map<Key<?>, Binding<?>> getExplicitBindingsThisLevel();

    void putBinding(Key<?> key, BindingImpl<?> binding);

    
    Scope getScope(Class<? extends Annotation> scopingAnnotation);

    void putAnnotation(Class<? extends Annotation> annotationType, Scope scope);

    void addConverter(MatcherAndConverter matcherAndConverter);

    
    MatcherAndConverter getConverter(
            String stringValue, TypeLiteral<?> type, Errors errors, Object source);

    
    Iterable<MatcherAndConverter> getConvertersThisLevel();

    void addTypeListener(TypeListenerBinding typeListenerBinding);

    List<TypeListenerBinding> getTypeListenerBindings();

    
    void blacklist(Key<?> key);

    
    boolean isBlacklisted(Key<?> key);

    
    Object lock();

        void clearBlacklisted();

    void makeAllBindingsToEagerSingletons(Injector injector);
}
