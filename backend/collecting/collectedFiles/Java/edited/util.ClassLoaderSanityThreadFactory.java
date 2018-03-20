package hudson.util;

import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;


public class ClassLoaderSanityThreadFactory implements ThreadFactory {
    private final ThreadFactory delegate;

    public ClassLoaderSanityThreadFactory(ThreadFactory delegate) {
        this.delegate = delegate;
    }

    @Override public Thread newThread(Runnable r) {
        Thread t = delegate.newThread(r);
        t.setContextClassLoader(ClassLoaderSanityThreadFactory.class.getClassLoader());
        return t;
    }
}
