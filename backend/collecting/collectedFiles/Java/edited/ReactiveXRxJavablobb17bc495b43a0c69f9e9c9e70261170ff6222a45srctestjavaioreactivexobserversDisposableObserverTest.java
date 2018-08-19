

package io.reactivex.observers;

import static org.junit.Assert.*;

import java.util.*;

import org.junit.Test;

import io.reactivex.Observable;
import io.reactivex.TestHelper;
import io.reactivex.disposables.*;
import io.reactivex.internal.util.EndConsumerHelper;
import io.reactivex.plugins.RxJavaPlugins;

public class DisposableObserverTest {

    static final class TestDisposableObserver<T> extends DisposableObserver<T> {

        int start;

        final List<T> values = new ArrayList<T>();

        final List<Throwable> errors = new ArrayList<Throwable>();

        int completions;

        @Override
        protected void onStart() {
            super.onStart();

            start++;
        }

        @Override
        public void onNext(T value) {
            values.add(value);
        }

        @Override
        public void onError(Throwable e) {
            errors.add(e);
        }

        @Override
        public void onComplete() {
            completions++;
        }
    }

    @Test
    public void normal() {
        TestDisposableObserver<Integer> tc = new TestDisposableObserver<Integer>();

        assertFalse(tc.isDisposed());
        assertEquals(0, tc.start);
        assertTrue(tc.values.isEmpty());
        assertTrue(tc.errors.isEmpty());

        Observable.just(1).subscribe(tc);

        assertFalse(tc.isDisposed());
        assertEquals(1, tc.start);
        assertEquals(1, tc.values.get(0).intValue());
        assertTrue(tc.errors.isEmpty());
    }

    @Test
    public void startOnce() {

        List<Throwable> error = TestHelper.trackPluginErrors();

        try {
            TestDisposableObserver<Integer> tc = new TestDisposableObserver<Integer>();

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
        TestDisposableObserver<Integer> tc = new TestDisposableObserver<Integer>();

        assertFalse(tc.isDisposed());

        tc.dispose();

        assertTrue(tc.isDisposed());

        Disposable d = Disposables.empty();

        tc.onSubscribe(d);

        assertTrue(d.isDisposed());

        assertEquals(0, tc.start);
    }
}
