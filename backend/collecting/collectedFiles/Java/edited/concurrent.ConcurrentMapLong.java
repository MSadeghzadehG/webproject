

package org.elasticsearch.common.util.concurrent;

import java.util.concurrent.ConcurrentMap;

public interface ConcurrentMapLong<T> extends ConcurrentMap<Long, T> {

    T get(long key);

    T remove(long key);

    T put(long key, T value);

    T putIfAbsent(long key, T value);
}
