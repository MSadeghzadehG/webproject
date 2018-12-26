

package io.reactivex.internal.util;

import java.util.*;

import io.reactivex.functions.Function;

public final class SorterFunction<T> implements Function<List<T>, List<T>> {

    final Comparator<? super T> comparator;

    public SorterFunction(Comparator<? super T> comparator) {
        this.comparator = comparator;
    }

    @Override
    public List<T> apply(List<T> t) throws Exception {
        Collections.sort(t, comparator);
        return t;
    }
}
