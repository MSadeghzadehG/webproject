

package io.reactivex.internal.observers;

import static org.junit.Assert.*;

import java.util.List;

import org.junit.Test;

import io.reactivex.TestHelper;
import io.reactivex.disposables.Disposables;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.Action;
import io.reactivex.internal.functions.Functions;
import io.reactivex.observers.TestObserver;
import io.reactivex.plugins.RxJavaPlugins;

public class DisposableLambdaObserverTest {

    @Test
    public void doubleOnSubscribe() {
        TestHelper.doubleOnSubscribe(new DisposableLambdaObserver<Integer>(
                new TestObserver<Integer>(), Functions.emptyConsumer(), Functions.EMPTY_ACTION
        ));
    }

    @Test
    public void disposeCrash() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            DisposableLambdaObserver<Integer> o = new DisposableLambdaObserver<Integer>(
                    new TestObserver<Integer>(), Functions.emptyConsumer(),
                    new Action() {
                        @Override
                        public void run() throws Exception {
                            throw new TestException();
                        }
                    }
            );

            o.onSubscribe(Disposables.empty());

            assertFalse(o.isDisposed());

            o.dispose();

            assertTrue(o.isDisposed());

            TestHelper.assertUndeliverable(errors, 0, TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
    }
}
