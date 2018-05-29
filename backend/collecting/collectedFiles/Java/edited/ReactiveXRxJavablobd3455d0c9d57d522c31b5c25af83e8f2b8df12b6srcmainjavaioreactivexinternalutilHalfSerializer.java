
package io.reactivex.internal.util;

import java.util.concurrent.atomic.AtomicInteger;

import org.reactivestreams.Subscriber;

import io.reactivex.Observer;
import io.reactivex.plugins.RxJavaPlugins;


public final class HalfSerializer {
    
    private HalfSerializer() {
        throw new IllegalStateException("No instances!");
    }

    
    public static <T> void onNext(Subscriber<? super T> subscriber, T value,
            AtomicInteger wip, AtomicThrowable error) {
        if (wip.get() == 0 && wip.compareAndSet(0, 1)) {
            subscriber.onNext(value);
            if (wip.decrementAndGet() != 0) {
                Throwable ex = error.terminate();
                if (ex != null) {
                    subscriber.onError(ex);
                } else {
                    subscriber.onComplete();
                }
            }
        }
    }

    
    public static void onError(Subscriber<?> subscriber, Throwable ex,
            AtomicInteger wip, AtomicThrowable error) {
        if (error.addThrowable(ex)) {
            if (wip.getAndIncrement() == 0) {
                subscriber.onError(error.terminate());
            }
        } else {
            RxJavaPlugins.onError(ex);
        }
    }


    
    public static void onComplete(Subscriber<?> subscriber, AtomicInteger wip, AtomicThrowable error) {
        if (wip.getAndIncrement() == 0) {
            Throwable ex = error.terminate();
            if (ex != null) {
                subscriber.onError(ex);
            } else {
                subscriber.onComplete();
            }
        }
    }

    
    public static <T> void onNext(Observer<? super T> observer, T value,
            AtomicInteger wip, AtomicThrowable error) {
        if (wip.get() == 0 && wip.compareAndSet(0, 1)) {
            observer.onNext(value);
            if (wip.decrementAndGet() != 0) {
                Throwable ex = error.terminate();
                if (ex != null) {
                    observer.onError(ex);
                } else {
                    observer.onComplete();
                }
            }
        }
    }

    
    public static void onError(Observer<?> observer, Throwable ex,
            AtomicInteger wip, AtomicThrowable error) {
        if (error.addThrowable(ex)) {
            if (wip.getAndIncrement() == 0) {
                observer.onError(error.terminate());
            }
        } else {
            RxJavaPlugins.onError(ex);
        }
    }

    
    public static void onComplete(Observer<?> observer, AtomicInteger wip, AtomicThrowable error) {
        if (wip.getAndIncrement() == 0) {
            Throwable ex = error.terminate();
            if (ex != null) {
                observer.onError(ex);
            } else {
                observer.onComplete();
            }
        }
    }

}
