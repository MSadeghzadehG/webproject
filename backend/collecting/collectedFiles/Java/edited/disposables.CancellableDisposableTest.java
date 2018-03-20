

package io.reactivex.internal.disposables;

import static org.junit.Assert.*;

import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import org.junit.Test;

import io.reactivex.TestHelper;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.Cancellable;
import io.reactivex.plugins.RxJavaPlugins;

public class CancellableDisposableTest {

    @Test
    public void normal() {
        final AtomicInteger count = new AtomicInteger();

        Cancellable c = new Cancellable() {
            @Override
            public void cancel() throws Exception {
                count.getAndIncrement();
            }
        };

        CancellableDisposable cd = new CancellableDisposable(c);

        assertFalse(cd.isDisposed());

        cd.dispose();
        cd.dispose();

        assertTrue(cd.isDisposed());

        assertEquals(1, count.get());
    }

    @Test
    public void cancelThrows() {
        final AtomicInteger count = new AtomicInteger();

        Cancellable c = new Cancellable() {
            @Override
            public void cancel() throws Exception {
                count.getAndIncrement();
                throw new TestException();
            }
        };

        CancellableDisposable cd = new CancellableDisposable(c);

        assertFalse(cd.isDisposed());

        List<Throwable> list = TestHelper.trackPluginErrors();
        try {
            cd.dispose();
            cd.dispose();

            TestHelper.assertUndeliverable(list, 0, TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
        assertTrue(cd.isDisposed());

        assertEquals(1, count.get());
    }

    @Test
    public void disposeRace() {

        for (int i = 0; i < TestHelper.RACE_DEFAULT_LOOPS; i++) {
            final AtomicInteger count = new AtomicInteger();

            Cancellable c = new Cancellable() {
                @Override
                public void cancel() throws Exception {
                    count.getAndIncrement();
                }
            };

            final CancellableDisposable cd = new CancellableDisposable(c);

            Runnable r = new Runnable() {
                @Override
                public void run() {
                    cd.dispose();
                }
            };

            TestHelper.race(r, r);

            assertEquals(1, count.get());
        }
    }

}
