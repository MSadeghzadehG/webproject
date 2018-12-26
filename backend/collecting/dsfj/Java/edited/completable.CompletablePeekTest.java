

package io.reactivex.internal.operators.completable;

import java.util.List;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.Action;
import io.reactivex.internal.functions.Functions;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.subjects.CompletableSubject;

public class CompletablePeekTest {

    @Test
    public void onAfterTerminateCrashes() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            Completable.complete()
            .doAfterTerminate(new Action() {
                @Override
                public void run() throws Exception {
                    throw new TestException();
                }
            })
            .test()
            .assertResult();

            TestHelper.assertUndeliverable(errors, 0, TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void disposed() {
        TestHelper.checkDisposed(CompletableSubject.create().doOnComplete(Functions.EMPTY_ACTION));
    }
}
