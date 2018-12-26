

package io.reactivex.internal.operators.single;

import io.reactivex.Single;
import io.reactivex.SingleSource;
import io.reactivex.TestHelper;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.Action;
import io.reactivex.functions.Function;
import io.reactivex.internal.functions.Functions;
import io.reactivex.observers.TestObserver;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.subjects.PublishSubject;
import org.junit.Test;

import java.util.List;

import static org.junit.Assert.assertEquals;

public class SingleDoAfterTerminateTest {

    private final int[] call = { 0 };

    private final Action afterTerminate = new Action() {
        @Override
        public void run() throws Exception {
            call[0]++;
        }
    };

    private final TestObserver<Integer> to = new TestObserver<Integer>();

    @Test
    public void just() {
        Single.just(1)
        .doAfterTerminate(afterTerminate)
        .subscribeWith(to)
        .assertResult(1);

        assertAfterTerminateCalledOnce();
    }

    @Test
    public void error() {
        Single.<Integer>error(new TestException())
        .doAfterTerminate(afterTerminate)
        .subscribeWith(to)
        .assertFailure(TestException.class);

        assertAfterTerminateCalledOnce();
    }

    @Test(expected = NullPointerException.class)
    public void afterTerminateActionNull() {
        Single.just(1).doAfterTerminate(null);
    }

    @Test
    public void justConditional() {
        Single.just(1)
        .doAfterTerminate(afterTerminate)
        .filter(Functions.alwaysTrue())
        .subscribeWith(to)
        .assertResult(1);

        assertAfterTerminateCalledOnce();
    }

    @Test
    public void errorConditional() {
        Single.<Integer>error(new TestException())
        .doAfterTerminate(afterTerminate)
        .filter(Functions.alwaysTrue())
        .subscribeWith(to)
        .assertFailure(TestException.class);

        assertAfterTerminateCalledOnce();
    }

    @Test
    public void actionThrows() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            Single.just(1)
            .doAfterTerminate(new Action() {
                @Override
                public void run() throws Exception {
                    throw new TestException();
                }
            })
            .test()
            .assertResult(1);

            TestHelper.assertUndeliverable(errors, 0, TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void dispose() {
        TestHelper.checkDisposed(PublishSubject.<Integer>create().singleOrError().doAfterTerminate(afterTerminate));
    }

    @Test
    public void doubleOnSubscribe() {
        TestHelper.checkDoubleOnSubscribeSingle(new Function<Single<Integer>, SingleSource<Integer>>() {
            @Override
            public SingleSource<Integer> apply(Single<Integer> m) throws Exception {
                return m.doAfterTerminate(afterTerminate);
            }
        });
    }

    private void assertAfterTerminateCalledOnce() {
        assertEquals(1, call[0]);
    }
}
