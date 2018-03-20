

package org.elasticsearch.common;

import java.util.function.BiConsumer;


@FunctionalInterface
public interface CheckedBiConsumer<T, U, E extends Exception> {
    void accept(T t, U u) throws E;
}
