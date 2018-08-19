

package io.reactivex.observers;

import static org.junit.Assert.*;

import java.util.*;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.disposables.*;
import io.reactivex.internal.util.EndConsumerHelper;
import io.reactivex.observers.DisposableCompletableObserver;
import io.reactivex.plugins.RxJavaPlugins;

public class DisposableCompletableObserverTest {

    static final class TestCompletable extends DisposableCompletableObserver {

        int start;

        int complete;

        final List<Throwable> errors = new ArrayList<Throwable>();

        @Override
        protected void onStart() {
            super.onStart();

            start++;
        }

        @Override
        public void onComplete() {
            complete++;
        }

        @Override
        public void onError(Throwable e) {
            errors.add(e);
        }

    }

    @Test
    public void normal() {
        TestCompletable tc = new TestCompletable();

        assertFalse(tc.isDisposed());
        assertEquals(0, tc.start);
        assertEquals(0, tc.complete);
        assertTrue(tc.errors.isEmpty());

        Completable.complete().subscribe(tc);

        assertFalse(tc.isDisposed());
        assertEquals(1, tc.start);
        assertEquals(1, tc.complete);
        assertTrue(tc.errors.isEmpty());
    }

    @Test
    public void startOnce() {

        List<Throwable> error = TestHelper.trackPluginErrors();

        try {
            TestCompletable tc = new TestCompletable();

            tc.onSubscribe(Disposables.empty());

            Disposable d = Disposables.empty();

            tc.onSubscribe(d);

            assertTrue(d.isDisposed());

            assertEquals(1, tc.start);

            TestHelper.assertError(error, 0, IllegalStateException.class, EndConsumerHelper.composeMessage(tc.getClass().getName()));
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void dispose() {
        TestCompletable tc = new TestCompletable();
        tc.dispose();

        assertTrue(tc.isDisposed());

        Disposable d = Disposables.empty();

        tc.onSubscribe(d);

        assertTrue(d.isDisposed());

        assertEquals(0, tc.start);
    }
}
