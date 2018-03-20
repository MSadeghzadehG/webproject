
package io.reactivex.internal.util;

import java.util.concurrent.atomic.AtomicLong;

import io.reactivex.plugins.RxJavaPlugins;


public final class BackpressureHelper {
    
    private BackpressureHelper() {
        throw new IllegalStateException("No instances!");
    }

    
    public static long addCap(long a, long b) {
        long u = a + b;
        if (u < 0L) {
            return Long.MAX_VALUE;
        }
        return u;
    }

    
    public static long multiplyCap(long a, long b) {
        long u = a * b;
        if (((a | b) >>> 31) != 0) {
            if (u / a != b) {
                return Long.MAX_VALUE;
            }
        }
        return u;
    }

    
    public static long add(AtomicLong requested, long n) {
        for (;;) {
            long r = requested.get();
            if (r == Long.MAX_VALUE) {
                return Long.MAX_VALUE;
            }
            long u = addCap(r, n);
            if (requested.compareAndSet(r, u)) {
                return r;
            }
        }
    }

    
    public static long addCancel(AtomicLong requested, long n) {
        for (;;) {
            long r = requested.get();
            if (r == Long.MIN_VALUE) {
                return Long.MIN_VALUE;
            }
            if (r == Long.MAX_VALUE) {
                return Long.MAX_VALUE;
            }
            long u = addCap(r, n);
            if (requested.compareAndSet(r, u)) {
                return r;
            }
        }
    }

    
    public static long produced(AtomicLong requested, long n) {
        for (;;) {
            long current = requested.get();
            if (current == Long.MAX_VALUE) {
                return Long.MAX_VALUE;
            }
            long update = current - n;
            if (update < 0L) {
                RxJavaPlugins.onError(new IllegalStateException("More produced than requested: " + update));
                update = 0L;
            }
            if (requested.compareAndSet(current, update)) {
                return update;
            }
        }
    }

    
    public static long producedCancel(AtomicLong requested, long n) {
        for (;;) {
            long current = requested.get();
            if (current == Long.MIN_VALUE) {
                return Long.MIN_VALUE;
            }
            if (current == Long.MAX_VALUE) {
                return Long.MAX_VALUE;
            }
            long update = current - n;
            if (update < 0L) {
                RxJavaPlugins.onError(new IllegalStateException("More produced than requested: " + update));
                update = 0L;
            }
            if (requested.compareAndSet(current, update)) {
                return update;
            }
        }
    }
}
