

package org.elasticsearch.common.inject.multibindings;

import org.elasticsearch.common.inject.Binder;
import org.elasticsearch.common.inject.Binding;
import org.elasticsearch.common.inject.ConfigurationException;
import org.elasticsearch.common.inject.Inject;
import org.elasticsearch.common.inject.Injector;
import org.elasticsearch.common.inject.Key;
import org.elasticsearch.common.inject.Module;
import org.elasticsearch.common.inject.Provider;
import org.elasticsearch.common.inject.TypeLiteral;
import org.elasticsearch.common.inject.binder.LinkedBindingBuilder;
import org.elasticsearch.common.inject.internal.Errors;
import org.elasticsearch.common.inject.spi.Dependency;
import org.elasticsearch.common.inject.spi.HasDependencies;
import org.elasticsearch.common.inject.spi.Message;
import org.elasticsearch.common.inject.util.Types;

import java.lang.annotation.Annotation;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

import static java.util.Collections.emptyList;
import static java.util.Collections.singleton;
import static java.util.Collections.unmodifiableSet;


public abstract class Multibinder<T> {
    private Multibinder() {
    }

    
    public static <T> Multibinder<T> newSetBinder(Binder binder, TypeLiteral<T> type) {
        binder = binder.skipSources(RealMultibinder.class, Multibinder.class);
        RealMultibinder<T> result = new RealMultibinder<>(binder, type, "",
                Key.get(Multibinder.<T>setOf(type)));
        binder.install(result);
        return result;
    }

    
    public static <T> Multibinder<T> newSetBinder(Binder binder, Class<T> type) {
        return newSetBinder(binder, TypeLiteral.get(type));
    }

    
    public static <T> Multibinder<T> newSetBinder(
            Binder binder, TypeLiteral<T> type, Annotation annotation) {
        binder = binder.skipSources(RealMultibinder.class, Multibinder.class);
        RealMultibinder<T> result = new RealMultibinder<>(binder, type, annotation.toString(),
                Key.get(Multibinder.<T>setOf(type), annotation));
        binder.install(result);
        return result;
    }

    
    public static <T> Multibinder<T> newSetBinder(
            Binder binder, Class<T> type, Annotation annotation) {
        return newSetBinder(binder, TypeLiteral.get(type), annotation);
    }

    
    public static <T> Multibinder<T> newSetBinder(Binder binder, TypeLiteral<T> type,
                                                  Class<? extends Annotation> annotationType) {
        binder = binder.skipSources(RealMultibinder.class, Multibinder.class);
        RealMultibinder<T> result = new RealMultibinder<>(binder, type, "@" + annotationType.getName(),
                Key.get(Multibinder.<T>setOf(type), annotationType));
        binder.install(result);
        return result;
    }

    
    public static <T> Multibinder<T> newSetBinder(Binder binder, Class<T> type,
                                                  Class<? extends Annotation> annotationType) {
        return newSetBinder(binder, TypeLiteral.get(type), annotationType);
    }

    @SuppressWarnings("unchecked")     private static <T> TypeLiteral<Set<T>> setOf(TypeLiteral<T> elementType) {
        Type type = Types.setOf(elementType.getType());
        return (TypeLiteral<Set<T>>) TypeLiteral.get(type);
    }

    
    public abstract LinkedBindingBuilder<T> addBinding();

    
    public static final class RealMultibinder<T> extends Multibinder<T>
            implements Module, Provider<Set<T>>, HasDependencies {

        private final TypeLiteral<T> elementType;
        private final String setName;
        private final Key<Set<T>> setKey;

        
        private Binder binder;

        
        private List<Provider<T>> providers;
        private Set<Dependency<?>> dependencies;

        private RealMultibinder(Binder binder, TypeLiteral<T> elementType,
                                String setName, Key<Set<T>> setKey) {
            this.binder = Objects.requireNonNull(binder, "binder");
            this.elementType = Objects.requireNonNull(elementType, "elementType");
            this.setName = Objects.requireNonNull(setName, "setName");
            this.setKey = Objects.requireNonNull(setKey, "setKey");
        }

        @Override
        @SuppressWarnings("unchecked")
        public void configure(Binder binder) {
            checkConfiguration(!isInitialized(), "Multibinder was already initialized");

            binder.bind(setKey).toProvider(this);
        }

        @Override
        public LinkedBindingBuilder<T> addBinding() {
            checkConfiguration(!isInitialized(), "Multibinder was already initialized");

            return binder.bind(Key.get(elementType, new RealElement(setName)));
        }

        
        @Inject
        public void initialize(Injector injector) {
            providers = new ArrayList<>();
            Set<Dependency<?>> dependencies = new HashSet<>();
            for (Binding<?> entry : injector.findBindingsByType(elementType)) {
                if (keyMatches(entry.getKey())) {
                    @SuppressWarnings("unchecked")                             Binding<T> binding = (Binding<T>) entry;
                    providers.add(binding.getProvider());
                    dependencies.add(Dependency.get(binding.getKey()));
                }
            }

            this.dependencies = unmodifiableSet(dependencies);
            this.binder = null;
        }

        private boolean keyMatches(Key<?> key) {
            return key.getTypeLiteral().equals(elementType)
                    && key.getAnnotation() instanceof Element
                    && ((Element) key.getAnnotation()).setName().equals(setName);
        }

        private boolean isInitialized() {
            return binder == null;
        }

        @Override
        public Set<T> get() {
            checkConfiguration(isInitialized(), "Multibinder is not initialized");

            Set<T> result = new LinkedHashSet<>();
            for (Provider<T> provider : providers) {
                final T newValue = provider.get();
                checkConfiguration(newValue != null, "Set injection failed due to null element");
                checkConfiguration(result.add(newValue),
                        "Set injection failed due to duplicated element \"%s\"", newValue);
            }
            return Collections.unmodifiableSet(result);
        }

        String getSetName() {
            return setName;
        }

        Key<Set<T>> getSetKey() {
            return setKey;
        }

        @Override
        public Set<Dependency<?>> getDependencies() {
            return dependencies;
        }

        @Override
        public boolean equals(Object o) {
            return o instanceof RealMultibinder
                    && ((RealMultibinder<?>) o).setKey.equals(setKey);
        }

        @Override
        public int hashCode() {
            return setKey.hashCode();
        }

        @Override
        public String toString() {
            return new StringBuilder()
                    .append(setName)
                    .append(setName.length() > 0 ? " " : "")
                    .append("Multibinder<")
                    .append(elementType)
                    .append(">")
                    .toString();
        }
    }

    static void checkConfiguration(boolean condition, String format, Object... args) {
        if (condition) {
            return;
        }

        throw new ConfigurationException(singleton(new Message(Errors.format(format, args))));
    }

    static <T> T checkNotNull(T reference, String name) {
        if (reference != null) {
            return reference;
        }

        NullPointerException npe = new NullPointerException(name);
        throw new ConfigurationException(singleton(
                new Message(emptyList(), npe)));
    }
}
