

package io.reactivex.exceptions;

import static org.junit.Assert.*;

import java.util.List;

import org.junit.*;

import io.reactivex.*;
import io.reactivex.internal.functions.Functions;
import io.reactivex.plugins.RxJavaPlugins;

public class OnErrorNotImplementedExceptionTest {

    List<Throwable> errors;

    @Before
    public void before() {
       errors = TestHelper.trackPluginErrors();
    }

    @After
    public void after() {
        RxJavaPlugins.reset();

        assertFalse("" + errors, errors.isEmpty());
        TestHelper.assertError(errors, 0, OnErrorNotImplementedException.class);
        Throwable c = errors.get(0).getCause();
        assertTrue("" + c, c instanceof TestException);
    }

    @Test
    public void flowableSubscribe0() {
        Flowable.error(new TestException())
        .subscribe();
    }

    @Test
    public void flowableSubscribe1() {
        Flowable.error(new TestException())
        .subscribe(Functions.emptyConsumer());
    }

    @Test
    public void flowableForEachWhile() {
        Flowable.error(new TestException())
        .forEachWhile(Functions.alwaysTrue());
    }

    @Test
    public void flowableBlockingSubscribe1() {
        Flowable.error(new TestException())
        .blockingSubscribe(Functions.emptyConsumer());
    }

    @Test
    public void observableSubscribe0() {
        Observable.error(new TestException())
        .subscribe();
    }

    @Test
    public void observableSubscribe1() {
        Observable.error(new TestException())
        .subscribe(Functions.emptyConsumer());
    }

    @Test
    public void observableForEachWhile() {
        Observable.error(new TestException())
        .forEachWhile(Functions.alwaysTrue());
    }

    @Test
    public void observableBlockingSubscribe1() {
        Observable.error(new TestException())
        .blockingSubscribe(Functions.emptyConsumer());
    }

    @Test
    public void singleSubscribe0() {
        Single.error(new TestException())
        .subscribe();
    }

    @Test
    public void singleSubscribe1() {
        Single.error(new TestException())
        .subscribe(Functions.emptyConsumer());
    }


    @Test
    public void maybeSubscribe0() {
        Maybe.error(new TestException())
        .subscribe();
    }

    @Test
    public void maybeSubscribe1() {
        Maybe.error(new TestException())
        .subscribe(Functions.emptyConsumer());
    }

    @Test
    public void completableSubscribe0() {
        Completable.error(new TestException())
        .subscribe();
    }

    @Test
    public void completableSubscribe1() {
        Completable.error(new TestException())
        .subscribe(Functions.EMPTY_ACTION);
    }

}