

package io.reactivex.internal.operators.maybe;

import static org.junit.Assert.assertTrue;

import java.util.concurrent.*;

import org.junit.Test;

import io.reactivex.Maybe;
import io.reactivex.exceptions.TestException;
import io.reactivex.internal.functions.Functions;
import io.reactivex.observers.TestObserver;
import io.reactivex.schedulers.Schedulers;

public class MaybeFromFutureTest {

    @Test
    public void cancelImmediately() {
        FutureTask<Integer> ft = new FutureTask<Integer>(Functions.justCallable(1));

        Maybe.fromFuture(ft).test(true)
        .assertEmpty();
    }

    @Test
    public void timeout() {
        FutureTask<Integer> ft = new FutureTask<Integer>(Functions.justCallable(1));

        Maybe.fromFuture(ft, 1, TimeUnit.MILLISECONDS).test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertFailure(TimeoutException.class);
    }

    @Test
    public void timedWait() {
        FutureTask<Integer> ft = new FutureTask<Integer>(Functions.justCallable(1));
        ft.run();

        Maybe.fromFuture(ft, 1, TimeUnit.MILLISECONDS).test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertResult(1);
    }

    @Test
    public void interrupt() {
        FutureTask<Integer> ft = new FutureTask<Integer>(Functions.justCallable(1));

        Thread.currentThread().interrupt();

        Maybe.fromFuture(ft, 1, TimeUnit.MILLISECONDS).test()
        .assertFailure(InterruptedException.class);
    }

    @Test
    public void cancelWhileRunning() {
        final TestObserver<Object> to = new TestObserver<Object>();

        FutureTask<Object> ft = new FutureTask<Object>(new Runnable() {
            @Override
            public void run() {
                to.cancel();
            }
        }, null);

        Schedulers.single().scheduleDirect(ft, 100, TimeUnit.MILLISECONDS);

        Maybe.fromFuture(ft)
        .subscribeWith(to)
        .assertEmpty();

        assertTrue(to.isDisposed());
    }

    @Test
    public void cancelAndCrashWhileRunning() {
        final TestObserver<Object> to = new TestObserver<Object>();

        FutureTask<Object> ft = new FutureTask<Object>(new Runnable() {
            @Override
            public void run() {
                to.cancel();
                throw new TestException();
            }
        }, null);

        Schedulers.single().scheduleDirect(ft, 100, TimeUnit.MILLISECONDS);

        Maybe.fromFuture(ft)
        .subscribeWith(to)
        .assertEmpty();

        assertTrue(to.isDisposed());
    }

    @Test
    public void futureNull() {
        FutureTask<Object> ft = new FutureTask<Object>(new Runnable() {
            @Override
            public void run() {
            }
        }, null);

        Schedulers.single().scheduleDirect(ft, 100, TimeUnit.MILLISECONDS);

        Maybe.fromFuture(ft)
        .test()
        .assertResult();
    }
}
