

package org.elasticsearch.common.util;

import org.apache.lucene.util.Accountable;

import java.util.Collection;
import java.util.Collections;
import java.util.concurrent.atomic.AtomicBoolean;


abstract class AbstractArray implements BigArray {

    private final BigArrays bigArrays;
    public final boolean clearOnResize;
    private final AtomicBoolean closed = new AtomicBoolean(false);

    AbstractArray(BigArrays bigArrays, boolean clearOnResize) {
        this.bigArrays = bigArrays;
        this.clearOnResize = clearOnResize;
    }

    @Override
    public final void close() {
        if (closed.compareAndSet(false, true)) {
            try {
                bigArrays.adjustBreaker(-ramBytesUsed(), true);
            } finally {
                doClose();
            }
        }
    }

    protected abstract void doClose();

    @Override
    public Collection<Accountable> getChildResources() {
        return Collections.emptyList();
    }
}
