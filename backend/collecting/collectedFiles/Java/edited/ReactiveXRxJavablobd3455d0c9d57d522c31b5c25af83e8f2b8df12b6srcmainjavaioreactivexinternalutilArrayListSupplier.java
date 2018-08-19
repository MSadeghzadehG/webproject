

package io.reactivex.internal.util;

import java.util.*;
import java.util.concurrent.Callable;

import io.reactivex.functions.Function;

public enum ArrayListSupplier implements Callable<List<Object>>, Function<Object, List<Object>> {
    INSTANCE;

    @SuppressWarnings({ "unchecked", "rawtypes" })
    public static <T> Callable<List<T>> asCallable() {
        return (Callable)INSTANCE;
    }

    @SuppressWarnings({ "unchecked", "rawtypes" })
    public static <T, O> Function<O, List<T>> asFunction() {
        return (Function)INSTANCE;
    }

    @Override
    public List<Object> call() throws Exception {
        return new ArrayList<Object>();
    }

    @Override public List<Object> apply(Object o) throws Exception {
        return new ArrayList<Object>();
    }
}
