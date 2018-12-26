

package org.elasticsearch.common.recycler;

import org.elasticsearch.common.util.concurrent.ConcurrentCollections;

import java.util.Deque;
import java.util.concurrent.atomic.AtomicInteger;


public class ConcurrentDequeRecycler<T> extends DequeRecycler<T> {

        final AtomicInteger size;

    public ConcurrentDequeRecycler(C<T> c, int maxSize) {
        super(c, ConcurrentCollections.<T>newDeque(), maxSize);
        this.size = new AtomicInteger();
    }

    @Override
    public void close() {
        assert deque.size() == size.get();
        super.close();
        size.set(0);
    }

    @Override
    public V<T> obtain(int sizing) {
        final V<T> v = super.obtain(sizing);
        if (v.isRecycled()) {
            size.decrementAndGet();
        }
        return v;
    }

    @Override
    protected boolean beforeRelease() {
        return size.incrementAndGet() <= maxSize;
    }

    @Override
    protected void afterRelease(boolean recycled) {
        if (!recycled) {
            size.decrementAndGet();
        }
    }

}
