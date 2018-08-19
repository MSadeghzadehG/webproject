
package io.reactivex.internal.util;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;

import io.reactivex.functions.BiConsumer;
import io.reactivex.functions.Consumer;

public final class TestingHelper {

    private TestingHelper() {
            }

    public static <T> Consumer<T> addToList(final List<T> list) {
        return new Consumer<T>() {

            @Override
            public void accept(T t) {
                list.add(t);
            }
        };
    }

    public static <T> Callable<List<T>> callableListCreator() {
        return new Callable<List<T>>() {

            @Override
            public List<T> call() {
                return new ArrayList<T>();
            }
        };
    }

    public static BiConsumer<Object, Object> biConsumerThrows(final RuntimeException e) {
        return new BiConsumer<Object, Object>() {

            @Override
            public void accept(Object t1, Object t2) {
                throw e;
            }
        };
    }
}
