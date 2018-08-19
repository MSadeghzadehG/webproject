

package io.reactivex.observers;

import static org.junit.Assert.*;

import java.util.*;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.disposables.*;
import io.reactivex.internal.util.EndConsumerHelper;
import io.reactivex.observers.DisposableMaybeObserver;
import io.reactivex.plugins.RxJavaPlugins;

public class DisposableMaybeObserverTest {

    static final class TestMaybe<T> extends DisposableMaybeObserver<T> {

        int start;

        final List<T> values = new ArrayList<T>();

        final List<Throwable> errors = new ArrayList<Throwable>();

        int complete;

        @Override
        protected void onStart() {
            super.onStart();

            start++;
        }

        @Override
        public void onSuccess(T value) {
            values.add(value);
        }

        @Override
        public void onError(Throwable e) {
            errors.add(e);
        }

        @Override
        public void onComplete() {
            complete++;
        }
    }

    @Test
    public void normal() {
        TestMaybe<Integer> tc = new TestMaybe<Integer>();

        assertFalse(tc.isDisposed());
        assertEquals(0, tc.start);
        assertTrue(tc.values.isEmpty());
        assertTrue(tc.errors.isEmpty());
        assertEquals(0, tc.complete);

        Maybe.just(1).subscribe(tc);

        assertFalse(tc.isDisposed());
        assertEquals(1, tc.start);
        assertEquals(1, tc.values.get(0).intValue());
        assertTrue(tc.errors.isEmpty());
        assertEquals(0, tc.complete);
    }

    @Test
    public void startOnce() {

        List<Throwable> error = TestHelper.trackPluginErrors();

        try {
            TestMaybe<Integer> tc = new TestMaybe<Integer>();

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
        TestMaybe<Integer> tc = new TestMaybe<Integer>();
        tc.dispose();

        assertTrue(tc.isDisposed());

        Disposable d = Disposables.empty();

        tc.onSubscribe(d);

        assertTrue(d.isDisposed());

        assertEquals(0, tc.start);
    }
}
