

package io.reactivex.internal.util;

import java.util.List;

import io.reactivex.functions.*;

@SuppressWarnings("rawtypes")
public enum ListAddBiConsumer implements BiFunction<List, Object, List> {
    INSTANCE;

    @SuppressWarnings("unchecked")
    public static <T> BiFunction<List<T>, T, List<T>> instance() {
        return (BiFunction)INSTANCE;
    }

    @SuppressWarnings("unchecked")
    @Override
    public List apply(List t1, Object t2) throws Exception {
        t1.add(t2);
        return t1;
    }
}
