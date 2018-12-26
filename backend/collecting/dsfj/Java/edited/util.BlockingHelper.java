

package io.reactivex.internal.util;

import java.util.concurrent.CountDownLatch;

import io.reactivex.disposables.Disposable;
import io.reactivex.internal.schedulers.NonBlockingThread;
import io.reactivex.plugins.RxJavaPlugins;


public final class BlockingHelper {
    
    private BlockingHelper() {
        throw new IllegalStateException("No instances!");
    }

    public static void awaitForComplete(CountDownLatch latch, Disposable subscription) {
        if (latch.getCount() == 0) {
                                    return;
        }
                try {
            verifyNonBlocking();
            latch.await();
        } catch (InterruptedException e) {
            subscription.dispose();
                                    Thread.currentThread().interrupt();
                        throw new IllegalStateException("Interrupted while waiting for subscription to complete.", e);
        }
    }

    
    public static void verifyNonBlocking() {
        if (RxJavaPlugins.isFailOnNonBlockingScheduler()
                && (Thread.currentThread() instanceof NonBlockingThread
                        || RxJavaPlugins.onBeforeBlocking())) {
            throw new IllegalStateException("Attempt to block on a Scheduler " + Thread.currentThread().getName() + " that doesn't support blocking operators as they may lead to deadlock");
        }
    }
}
