

package io.reactivex.internal.subscriptions;

import java.util.concurrent.atomic.*;

import org.reactivestreams.Subscription;

import io.reactivex.exceptions.ProtocolViolationException;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.util.BackpressureHelper;
import io.reactivex.plugins.RxJavaPlugins;


public enum SubscriptionHelper implements Subscription {
    
    CANCELLED
    ;

    @Override
    public void request(long n) {
            }

    @Override
    public void cancel() {
            }

    
    public static boolean validate(Subscription current, Subscription next) {
        if (next == null) {
            RxJavaPlugins.onError(new NullPointerException("next is null"));
            return false;
        }
        if (current != null) {
            next.cancel();
            reportSubscriptionSet();
            return false;
        }
        return true;
    }

    
    public static void reportSubscriptionSet() {
        RxJavaPlugins.onError(new ProtocolViolationException("Subscription already set!"));
    }

    
    public static boolean validate(long n) {
        if (n <= 0) {
            RxJavaPlugins.onError(new IllegalArgumentException("n > 0 required but it was " + n));
            return false;
        }
        return true;
    }

    
    public static void reportMoreProduced(long n) {
        RxJavaPlugins.onError(new ProtocolViolationException("More produced than requested: " + n));
    }
    
    public static boolean isCancelled(Subscription s) {
        return s == CANCELLED;
    }

    
    public static boolean set(AtomicReference<Subscription> field, Subscription s) {
        for (;;) {
            Subscription current = field.get();
            if (current == CANCELLED) {
                if (s != null) {
                    s.cancel();
                }
                return false;
            }
            if (field.compareAndSet(current, s)) {
                if (current != null) {
                    current.cancel();
                }
                return true;
            }
        }
    }

    
    public static boolean setOnce(AtomicReference<Subscription> field, Subscription s) {
        ObjectHelper.requireNonNull(s, "s is null");
        if (!field.compareAndSet(null, s)) {
            s.cancel();
            if (field.get() != CANCELLED) {
                reportSubscriptionSet();
            }
            return false;
        }
        return true;
    }

    
    public static boolean replace(AtomicReference<Subscription> field, Subscription s) {
        for (;;) {
            Subscription current = field.get();
            if (current == CANCELLED) {
                if (s != null) {
                    s.cancel();
                }
                return false;
            }
            if (field.compareAndSet(current, s)) {
                return true;
            }
        }
    }

    
    public static boolean cancel(AtomicReference<Subscription> field) {
        Subscription current = field.get();
        if (current != CANCELLED) {
            current = field.getAndSet(CANCELLED);
            if (current != CANCELLED) {
                if (current != null) {
                    current.cancel();
                }
                return true;
            }
        }
        return false;
    }

    
    public static boolean deferredSetOnce(AtomicReference<Subscription> field, AtomicLong requested,
            Subscription s) {
        if (SubscriptionHelper.setOnce(field, s)) {
            long r = requested.getAndSet(0L);
            if (r != 0L) {
                s.request(r);
            }
            return true;
        }
        return false;
    }

    
    public static void deferredRequest(AtomicReference<Subscription> field, AtomicLong requested, long n) {
        Subscription s = field.get();
        if (s != null) {
            s.request(n);
        } else {
            if (SubscriptionHelper.validate(n)) {
                BackpressureHelper.add(requested, n);

                s = field.get();
                if (s != null) {
                    long r = requested.getAndSet(0L);
                    if (r != 0L) {
                        s.request(r);
                    }
                }
            }
        }
    }

    
    public static boolean setOnce(AtomicReference<Subscription> field, Subscription s, long request) {
        if (setOnce(field, s)) {
            s.request(request);
            return true;
        }
        return false;
    }
}
