

package io.reactivex.internal.operators.completable;

import static org.junit.Assert.assertEquals;

import java.util.concurrent.atomic.AtomicInteger;

import org.junit.Test;

import io.reactivex.Completable;
import io.reactivex.exceptions.TestException;

public class CompletableFromRunnableTest {
    @Test(expected = NullPointerException.class)
    public void fromRunnableNull() {
        Completable.fromRunnable(null);
    }

    @Test
    public void fromRunnable() {
        final AtomicInteger atomicInteger = new AtomicInteger();

        Completable.fromRunnable(new Runnable() {
            @Override
            public void run() {
                atomicInteger.incrementAndGet();
            }
        })
            .test()
            .assertResult();

        assertEquals(1, atomicInteger.get());
    }

    @Test
    public void fromRunnableTwice() {
        final AtomicInteger atomicInteger = new AtomicInteger();

        Runnable run = new Runnable() {
            @Override
            public void run() {
                atomicInteger.incrementAndGet();
            }
        };

        Completable.fromRunnable(run)
            .test()
            .assertResult();

        assertEquals(1, atomicInteger.get());

        Completable.fromRunnable(run)
            .test()
            .assertResult();

        assertEquals(2, atomicInteger.get());
    }

    @Test
    public void fromRunnableInvokesLazy() {
        final AtomicInteger atomicInteger = new AtomicInteger();

        Completable completable = Completable.fromRunnable(new Runnable() {
            @Override
            public void run() {
                atomicInteger.incrementAndGet();
            }
        });

        assertEquals(0, atomicInteger.get());

        completable
            .test()
            .assertResult();

        assertEquals(1, atomicInteger.get());
    }

    @Test
    public void fromRunnableThrows() {
        Completable.fromRunnable(new Runnable() {
            @Override
            public void run() {
                throw new UnsupportedOperationException();
            }
        })
            .test()
            .assertFailure(UnsupportedOperationException.class);
    }

    @Test
    public void fromRunnableDisposed() {
        final AtomicInteger calls = new AtomicInteger();
        Completable.fromRunnable(new Runnable() {
            @Override
            public void run() {
                calls.incrementAndGet();
            }
        })
        .test(true)
        .assertEmpty();

        assertEquals(1, calls.get());
    }

    @Test
    public void fromRunnableErrorsDisposed() {
        final AtomicInteger calls = new AtomicInteger();
        Completable.fromRunnable(new Runnable() {
            @Override
            public void run() {
                calls.incrementAndGet();
                throw new TestException();
            }
        })
        .test(true)
        .assertEmpty();

        assertEquals(1, calls.get());
    }
}
