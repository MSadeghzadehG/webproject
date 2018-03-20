

package org.elasticsearch.bootstrap;

import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.message.ParameterizedMessage;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.logging.Loggers;

import java.io.IOError;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.Objects;
import java.util.function.Supplier;

class ElasticsearchUncaughtExceptionHandler implements Thread.UncaughtExceptionHandler {

    private final Supplier<String> loggingPrefixSupplier;

    ElasticsearchUncaughtExceptionHandler(final Supplier<String> loggingPrefixSupplier) {
        this.loggingPrefixSupplier = Objects.requireNonNull(loggingPrefixSupplier);
    }

    @Override
    public void uncaughtException(Thread t, Throwable e) {
        if (isFatalUncaught(e)) {
            try {
                onFatalUncaught(t.getName(), e);
            } finally {
                                                if (e instanceof InternalError) {
                    halt(128);
                } else if (e instanceof OutOfMemoryError) {
                    halt(127);
                } else if (e instanceof StackOverflowError) {
                    halt(126);
                } else if (e instanceof UnknownError) {
                    halt(125);
                } else if (e instanceof IOError) {
                    halt(124);
                } else {
                    halt(1);
                }
            }
        } else {
            onNonFatalUncaught(t.getName(), e);
        }
    }

    static boolean isFatalUncaught(Throwable e) {
        return e instanceof Error;
    }

    void onFatalUncaught(final String threadName, final Throwable t) {
        final Logger logger = Loggers.getLogger(ElasticsearchUncaughtExceptionHandler.class, loggingPrefixSupplier.get());
        logger.error(
            (org.apache.logging.log4j.util.Supplier<?>)
                () -> new ParameterizedMessage("fatal error in thread [{}], exiting", threadName), t);
    }

    void onNonFatalUncaught(final String threadName, final Throwable t) {
        final Logger logger = Loggers.getLogger(ElasticsearchUncaughtExceptionHandler.class, loggingPrefixSupplier.get());
        logger.warn((org.apache.logging.log4j.util.Supplier<?>)
            () -> new ParameterizedMessage("uncaught exception in thread [{}]", threadName), t);
    }

    void halt(int status) {
        AccessController.doPrivileged(new PrivilegedHaltAction(status));
    }

    static class PrivilegedHaltAction implements PrivilegedAction<Void> {

        private final int status;

        private PrivilegedHaltAction(final int status) {
            this.status = status;
        }

        @SuppressForbidden(reason = "halt")
        @Override
        public Void run() {
                        Runtime.getRuntime().halt(status);
            return null;
        }

    }

}
