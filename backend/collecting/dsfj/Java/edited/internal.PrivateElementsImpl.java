

package org.elasticsearch.common.inject.internal;

import org.elasticsearch.common.inject.Binder;
import org.elasticsearch.common.inject.Injector;
import org.elasticsearch.common.inject.Key;
import org.elasticsearch.common.inject.PrivateBinder;
import org.elasticsearch.common.inject.spi.Element;
import org.elasticsearch.common.inject.spi.ElementVisitor;
import org.elasticsearch.common.inject.spi.PrivateElements;

import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

import static java.util.Collections.unmodifiableMap;


public final class PrivateElementsImpl implements PrivateElements {

    

    private final Object source;

    private List<Element> elementsMutable = new ArrayList<>();
    private List<ExposureBuilder<?>> exposureBuilders = new ArrayList<>();

    
    private List<Element> elements;

    
    private Map<Key<?>, Object> exposedKeysToSources;
    private Injector injector;

    public PrivateElementsImpl(Object source) {
        this.source = Objects.requireNonNull(source, "source");
    }

    @Override
    public Object getSource() {
        return source;
    }

    @Override
    public List<Element> getElements() {
        if (elements == null) {
            elements = Collections.unmodifiableList(elementsMutable);
            elementsMutable = null;
        }

        return elements;
    }

    @Override
    public Injector getInjector() {
        return injector;
    }

    public void initInjector(Injector injector) {
        if (this.injector != null) {
            throw new IllegalStateException("injector already initialized");
        }
        this.injector = Objects.requireNonNull(injector, "injector");
    }

    @Override
    public Set<Key<?>> getExposedKeys() {
        if (exposedKeysToSources == null) {
            Map<Key<?>, Object> exposedKeysToSourcesMutable = new LinkedHashMap<>();
            for (ExposureBuilder<?> exposureBuilder : exposureBuilders) {
                exposedKeysToSourcesMutable.put(exposureBuilder.getKey(), exposureBuilder.getSource());
            }
            exposedKeysToSources = unmodifiableMap(exposedKeysToSourcesMutable);
            exposureBuilders = null;
        }

        return exposedKeysToSources.keySet();
    }

    @Override
    public <T> T acceptVisitor(ElementVisitor<T> visitor) {
        return visitor.visit(this);
    }

    public List<Element> getElementsMutable() {
        return elementsMutable;
    }

    public void addExposureBuilder(ExposureBuilder<?> exposureBuilder) {
        exposureBuilders.add(exposureBuilder);
    }

    @Override
    public void applyTo(Binder binder) {
        PrivateBinder privateBinder = binder.withSource(source).newPrivateBinder();

        for (Element element : getElements()) {
            element.applyTo(privateBinder);
        }

        getExposedKeys();         for (Map.Entry<Key<?>, Object> entry : exposedKeysToSources.entrySet()) {
            privateBinder.withSource(entry.getValue()).expose(entry.getKey());
        }
    }

    @Override
    public Object getExposedSource(Key<?> key) {
        getExposedKeys();         Object source = exposedKeysToSources.get(key);
        if (source == null) {
            throw new IllegalArgumentException(key + " not exposed by " + ".");
        }
        return source;
    }

    @Override
    public String toString() {
        return new ToStringBuilder(PrivateElements.class)
                .add("exposedKeys", getExposedKeys())
                .add("source", getSource())
                .toString();
    }
}
