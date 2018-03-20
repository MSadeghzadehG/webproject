

package io.reactivex.internal.operators.maybe;

import static org.junit.Assert.*;

import java.util.*;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.disposables.Disposables;
import io.reactivex.exceptions.TestException;
import io.reactivex.observers.TestObserver;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.processors.PublishProcessor;

public class MaybeAmbTest {

    @Test
    public void ambLots() {
        List<Maybe<Integer>> ms = new ArrayList<Maybe<Integer>>();

        for (int i = 0; i < 32; i++) {
            ms.add(Maybe.<Integer>never());
        }

        ms.add(Maybe.just(1));

        Maybe.amb(ms)
        .test()
        .assertResult(1);
    }

    @SuppressWarnings("unchecked")
    @Test
    public void ambFirstDone() {
        Maybe.amb(Arrays.asList(Maybe.just(1), Maybe.just(2)))
        .test()
        .assertResult(1);
    }

    @SuppressWarnings("unchecked")
    @Test
    public void dispose() {
        PublishProcessor<Integer> pp1 = PublishProcessor.create();
        PublishProcessor<Integer> pp2 = PublishProcessor.create();

        TestObserver<Integer> to = Maybe.amb(Arrays.asList(pp1.singleElement(), pp2.singleElement()))
        .test();

        assertTrue(pp1.hasSubscribers());
        assertTrue(pp2.hasSubscribers());

        to.dispose();

        assertFalse(pp1.hasSubscribers());
        assertFalse(pp2.hasSubscribers());
    }

    @SuppressWarnings("unchecked")
    @Test
    public void innerErrorRace() {
        for (int i = 0; i < TestHelper.RACE_DEFAULT_LOOPS; i++) {
            List<Throwable> errors = TestHelper.trackPluginErrors();
            try {
                final PublishProcessor<Integer> pp0 = PublishProcessor.create();
                final PublishProcessor<Integer> pp1 = PublishProcessor.create();

                final TestObserver<Integer> to = Maybe.amb(Arrays.asList(pp0.singleElement(), pp1.singleElement()))
                .test();

                final TestException ex = new TestException();

                Runnable r1 = new Runnable() {
                    @Override
                    public void run() {
                        pp0.onError(ex);
                    }
                };

                Runnable r2 = new Runnable() {
                    @Override
                    public void run() {
                        pp1.onError(ex);
                    }
                };

                TestHelper.race(r1, r2);

                to.assertFailure(TestException.class);

                if (!errors.isEmpty()) {
                    TestHelper.assertUndeliverable(errors, 0, TestException.class);
                }
            } finally {
                RxJavaPlugins.reset();
            }
        }
    }

    @Test
    public void disposeNoFurtherSignals() {
        @SuppressWarnings("unchecked")
        TestObserver<Integer> to = Maybe.ambArray(new Maybe<Integer>() {
            @Override
            protected void subscribeActual(
                    MaybeObserver<? super Integer> observer) {
                observer.onSubscribe(Disposables.empty());
                observer.onSuccess(1);
                observer.onSuccess(2);
                observer.onComplete();
            }
        }, Maybe.<Integer>never())
        .test();

        to.cancel();

        to.assertResult(1);
    }
}
