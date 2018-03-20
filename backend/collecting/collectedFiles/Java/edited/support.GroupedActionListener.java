
package org.elasticsearch.action.support;

import org.elasticsearch.action.ActionListener;
import org.elasticsearch.common.util.concurrent.AtomicArray;
import org.elasticsearch.common.util.concurrent.CountDown;

import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;


public final class GroupedActionListener<T> implements ActionListener<T> {
    private final CountDown countDown;
    private final AtomicInteger pos = new AtomicInteger();
    private final AtomicArray<T> results;
    private final ActionListener<Collection<T>> delegate;
    private final Collection<T> defaults;
    private final AtomicReference<Exception> failure = new AtomicReference<>();

    
    public GroupedActionListener(ActionListener<Collection<T>> delegate, int groupSize,
                                 Collection<T> defaults) {
        results = new AtomicArray<>(groupSize);
        countDown = new CountDown(groupSize);
        this.delegate = delegate;
        this.defaults = defaults;
    }

    @Override
    public void onResponse(T element) {
        results.setOnce(pos.incrementAndGet() - 1, element);
        if (countDown.countDown()) {
            if (failure.get() != null) {
                delegate.onFailure(failure.get());
            } else {
                List<T> collect = this.results.asList();
                collect.addAll(defaults);
                delegate.onResponse(Collections.unmodifiableList(collect));
            }
        }
    }

    @Override
    public void onFailure(Exception e) {
        if (failure.compareAndSet(null, e) == false) {
            failure.get().addSuppressed(e);
        }
        if (countDown.countDown()) {
            delegate.onFailure(failure.get());
        }
    }
}
