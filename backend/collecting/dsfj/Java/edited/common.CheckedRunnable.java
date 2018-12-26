

package org.elasticsearch.common;

import java.lang.Runnable;


@FunctionalInterface
public interface CheckedRunnable<E extends Exception> {
    void run() throws E;
}
