

package io.reactivex.internal.util;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Callable;

public enum HashMapSupplier implements Callable<Map<Object, Object>> {
    INSTANCE;

    @SuppressWarnings({ "unchecked", "rawtypes" })
    public static <K, V> Callable<Map<K, V>> asCallable() {
        return (Callable)INSTANCE;
    }

    @Override public Map<Object, Object> call() throws Exception {
        return new HashMap<Object, Object>();
    }
}
