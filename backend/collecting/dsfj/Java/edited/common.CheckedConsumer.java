

package org.elasticsearch.common;

import java.util.function.Consumer;


@FunctionalInterface
public interface CheckedConsumer<T, E extends Exception> {
    void accept(T t) throws E;
}
