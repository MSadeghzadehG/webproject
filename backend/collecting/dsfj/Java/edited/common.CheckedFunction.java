

package org.elasticsearch.common;

import java.util.function.Function;


@FunctionalInterface
public interface CheckedFunction<T, R, E extends Exception> {
    R apply(T t) throws E;
}
