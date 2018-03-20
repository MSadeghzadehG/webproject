package jenkins.util;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;


public class ContextResettingExecutorService extends InterceptingExecutorService {
    public ContextResettingExecutorService(ExecutorService base) {
        super(base);
    }

    @Override
    protected Runnable wrap(final Runnable r) {
        return new Runnable() {
            @Override
            public void run() {
                Thread t = Thread.currentThread();
                String name = t.getName();
                ClassLoader cl = t.getContextClassLoader();
                try {
                    r.run();
                } finally {
                    t.setName(name);
                    t.setContextClassLoader(cl);
                }
            }
        };
    }

    @Override
    protected <V> Callable<V> wrap(final Callable<V> r) {
        return new Callable<V>() {
            @Override
            public V call() throws Exception {
                Thread t = Thread.currentThread();
                String name = t.getName();
                ClassLoader cl = t.getContextClassLoader();
                try {
                    return r.call();
                } finally {
                    t.setName(name);
                    t.setContextClassLoader(cl);
                }
            }
        };
    }
}
