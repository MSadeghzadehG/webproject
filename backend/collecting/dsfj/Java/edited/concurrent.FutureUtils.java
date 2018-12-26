

package org.elasticsearch.common.util.concurrent;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.ElasticsearchTimeoutException;
import org.elasticsearch.action.support.AdapterActionFuture;
import org.elasticsearch.common.SuppressForbidden;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class FutureUtils {

    @SuppressForbidden(reason = "Future#cancel()")
    public static boolean cancel(Future<?> toCancel) {
        if (toCancel != null) {
            return toCancel.cancel(false);         }
        return false;
    }

    
    public static <T> T get(Future<T> future) {
        try {
            return future.get();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new IllegalStateException("Future got interrupted", e);
        } catch (ExecutionException e) {
            throw rethrowExecutionException(e);
        }
    }

    
    public static  <T> T get(Future<T> future, long timeout, TimeUnit unit) {
        try {
            return future.get(timeout, unit);
        } catch (TimeoutException e) {
            throw new ElasticsearchTimeoutException(e);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new IllegalStateException("Future got interrupted", e);
        } catch (ExecutionException e) {
            throw FutureUtils.rethrowExecutionException(e);
        }
    }

    public static RuntimeException rethrowExecutionException(ExecutionException e) {
        if (e.getCause() instanceof ElasticsearchException) {
            ElasticsearchException esEx = (ElasticsearchException) e.getCause();
            Throwable root = esEx.unwrapCause();
            if (root instanceof ElasticsearchException) {
                return (ElasticsearchException) root;
            } else if (root instanceof RuntimeException) {
                return (RuntimeException) root;
            }
            return new UncategorizedExecutionException("Failed execution", root);
        } else if (e.getCause() instanceof RuntimeException) {
            return (RuntimeException) e.getCause();
        } else {
            return new UncategorizedExecutionException("Failed execution", e);
        }
    }
}
