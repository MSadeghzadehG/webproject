

package io.reactivex.internal.operators.completable;

import static org.junit.Assert.assertEquals;

import java.util.concurrent.atomic.AtomicInteger;

import org.junit.Test;

import io.reactivex.Completable;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.Action;

public class CompletableFromActionTest {
    @Test(expected = NullPointerException.class)
    public void fromActionNull() {
        Completable.fromAction(null);
    }

    @Test
    public void fromAction() {
        final AtomicInteger atomicInteger = new AtomicInteger();

        Completable.fromAction(new Action() {
            @Override
            public void run() throws Exception {
                atomicInteger.incrementAndGet();
            }
        })
            .test()
            .assertResult();

        assertEquals(1, atomicInteger.get());
    }

    @Test
    public void fromActionTwice() {
        final AtomicInteger atomicInteger = new AtomicInteger();

        Action run = new Action() {
            @Override
            public void run() throws Exception {
                atomicInteger.incrementAndGet();
            }
        };

        Completable.fromAction(run)
            .test()
            .assertResult();

        assertEquals(1, atomicInteger.get());

        Completable.fromAction(run)
            .test()
            .assertResult();

        assertEquals(2, atomicInteger.get());
    }

    @Test
    public void fromActionInvokesLazy() {
        final AtomicInteger atomicInteger = new AtomicInteger();

        Completable completable = Completable.fromAction(new Action() {
            @Override
            public void run() throws Exception {
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
    public void fromActionThrows() {
        Completable.fromAction(new Action() {
            @Override
            public void run() throws Exception {
                throw new UnsupportedOperationException();
            }
        })
            .test()
            .assertFailure(UnsupportedOperationException.class);
    }

    @Test
    public void fromActionDisposed() {
        final AtomicInteger calls = new AtomicInteger();
        Completable.fromAction(new Action() {
            @Override
            public void run() throws Exception {
                calls.incrementAndGet();
            }
        })
        .test(true)
        .assertEmpty();

        assertEquals(1, calls.get());
    }

    @Test
    public void fromActionErrorsDisposed() {
        final AtomicInteger calls = new AtomicInteger();
        Completable.fromAction(new Action() {
            @Override
            public void run() throws Exception {
                calls.incrementAndGet();
                throw new TestException();
            }
        })
        .test(true)
        .assertEmpty();

        assertEquals(1, calls.get());
    }
}
