

package org.elasticsearch.common.inject.multibindings;

import java.lang.annotation.Annotation;
import java.util.concurrent.atomic.AtomicInteger;


class RealElement implements Element {
    private static final AtomicInteger nextUniqueId = new AtomicInteger(1);

    private final int uniqueId;
    private final String setName;

    RealElement(String setName) {
        uniqueId = nextUniqueId.getAndIncrement();
        this.setName = setName;
    }

    @Override
    public String setName() {
        return setName;
    }

    @Override
    public int uniqueId() {
        return uniqueId;
    }

    @Override
    public Class<? extends Annotation> annotationType() {
        return Element.class;
    }

    @Override
    public String toString() {
        return "@" + Element.class.getName() + "(setName=" + setName
                + ",uniqueId=" + uniqueId + ")";
    }

    @Override
    public boolean equals(Object o) {
        return o instanceof Element
                && ((Element) o).setName().equals(setName())
                && ((Element) o).uniqueId() == uniqueId();
    }

    @Override
    public int hashCode() {
        return 127 * ("setName".hashCode() ^ setName.hashCode())
                + 127 * ("uniqueId".hashCode() ^ uniqueId);
    }
}
