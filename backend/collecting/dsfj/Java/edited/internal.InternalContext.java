

package org.elasticsearch.common.inject.internal;

import org.elasticsearch.common.inject.spi.Dependency;

import java.util.HashMap;
import java.util.Map;


public final class InternalContext {

    private Map<Object, ConstructionContext<?>> constructionContexts = new HashMap<>();
    private Dependency dependency;

    @SuppressWarnings("unchecked")
    public <T> ConstructionContext<T> getConstructionContext(Object key) {
        ConstructionContext<T> constructionContext
                = (ConstructionContext<T>) constructionContexts.get(key);
        if (constructionContext == null) {
            constructionContext = new ConstructionContext<>();
            constructionContexts.put(key, constructionContext);
        }
        return constructionContext;
    }

    public Dependency getDependency() {
        return dependency;
    }

    public void setDependency(Dependency dependency) {
        this.dependency = dependency;
    }
}
