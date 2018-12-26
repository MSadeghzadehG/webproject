

package io.reactivex.internal.operators.maybe;

import io.reactivex.TestHelper;
import io.reactivex.disposables.*;
import io.reactivex.exceptions.*;
import io.reactivex.functions.*;
import io.reactivex.internal.functions.Functions;
import io.reactivex.plugins.RxJavaPlugins;
import org.junit.Test;

import java.util.List;

import static org.junit.Assert.*;

public class MaybeCallbackObserverTest {

    @Test
    public void dispose() {
        MaybeCallbackObserver<Object> mo = new MaybeCallbackObserver<Object>(Functions.emptyConsumer(), Functions.emptyConsumer(), Functions.EMPTY_ACTION);

        Disposable d = Disposables.empty();

        mo.onSubscribe(d);

        assertFalse(mo.isDisposed());

        mo.dispose();

        assertTrue(mo.isDisposed());

        assertTrue(d.isDisposed());
    }

    @Test
    public void onSuccessCrashes() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            MaybeCallbackObserver<Object> mo = new MaybeCallbackObserver<Object>(
                    new Consumer<Object>() {
                        @Override
                        public void accept(Object v) throws Exception {
                            throw new TestException();
                        }
                    },
                    Functions.emptyConsumer(),
                    Functions.EMPTY_ACTION);

            mo.onSubscribe(Disposables.empty());

            mo.onSuccess(1);

            TestHelper.assertUndeliverable(errors, 0, TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void onErrorCrashes() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            MaybeCallbackObserver<Object> mo = new MaybeCallbackObserver<Object>(
                    Functions.emptyConsumer(),
                    new Consumer<Object>() {
                        @Override
                        public void accept(Object v) throws Exception {
                            throw new TestException("Inner");
                        }
                    },
                    Functions.EMPTY_ACTION);

            mo.onSubscribe(Disposables.empty());

            mo.onError(new TestException("Outer"));

            TestHelper.assertError(errors, 0, CompositeException.class);

            List<Throwable> ce = TestHelper.compositeList(errors.get(0));

            TestHelper.assertError(ce, 0, TestException.class, "Outer");
            TestHelper.assertError(ce, 1, TestException.class, "Inner");
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void onCompleteCrashes() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            MaybeCallbackObserver<Object> mo = new MaybeCallbackObserver<Object>(
                    Functions.emptyConsumer(),
                    Functions.emptyConsumer(),
                    new Action() {
                        @Override
                        public void run() throws Exception {
                            throw new TestException();
                        }
                    });

            mo.onSubscribe(Disposables.empty());

            mo.onComplete();

            TestHelper.assertUndeliverable(errors, 0, TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void onErrorMissingShouldReportNoCustomOnError() {
        MaybeCallbackObserver<Integer> o = new MaybeCallbackObserver<Integer>(Functions.<Integer>emptyConsumer(),
                Functions.ON_ERROR_MISSING,
                Functions.EMPTY_ACTION);

        assertFalse(o.hasCustomOnError());
    }

    @Test
    public void customOnErrorShouldReportCustomOnError() {
        MaybeCallbackObserver<Integer> o = new MaybeCallbackObserver<Integer>(Functions.<Integer>emptyConsumer(),
                Functions.<Throwable>emptyConsumer(),
                Functions.EMPTY_ACTION);

        assertTrue(o.hasCustomOnError());
    }
}
