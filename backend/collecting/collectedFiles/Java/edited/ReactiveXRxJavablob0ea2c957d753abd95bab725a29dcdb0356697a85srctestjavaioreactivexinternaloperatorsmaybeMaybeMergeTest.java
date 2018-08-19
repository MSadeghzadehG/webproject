

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.schedulers.Schedulers;

public class MaybeMergeTest {

    @Test
    public void delayErrorWithMaxConcurrency() {
        Maybe.mergeDelayError(
                Flowable.just(Maybe.just(1), Maybe.just(2), Maybe.just(3)), 1)
        .test()
        .assertResult(1, 2, 3);
    }

    @Test
    public void delayErrorWithMaxConcurrencyError() {
        Maybe.mergeDelayError(
                Flowable.just(Maybe.just(1), Maybe.<Integer>error(new TestException()), Maybe.just(3)), 1)
        .test()
        .assertFailure(TestException.class, 1, 3);
    }

    @Test
    public void delayErrorWithMaxConcurrencyAsync() {
        final AtomicInteger count = new AtomicInteger();
        @SuppressWarnings("unchecked")
        Maybe<Integer>[] sources = new Maybe[3];
        for (int i = 0; i < 3; i++) {
            final int j = i + 1;
            sources[i] = Maybe.fromCallable(new Callable<Integer>() {
                @Override
                public Integer call() throws Exception {
                    return count.incrementAndGet() - j;
                }
            })
            .subscribeOn(Schedulers.io());
        }

        for (int i = 0; i < 1000; i++) {
            count.set(0);
            Maybe.mergeDelayError(
                    Flowable.fromArray(sources), 1)
            .test()
            .awaitDone(5, TimeUnit.SECONDS)
            .assertResult(0, 0, 0);
        }
    }

    @Test
    public void delayErrorWithMaxConcurrencyAsyncError() {
        final AtomicInteger count = new AtomicInteger();
        @SuppressWarnings("unchecked")
        Maybe<Integer>[] sources = new Maybe[3];
        for (int i = 0; i < 3; i++) {
            final int j = i + 1;
            sources[i] = Maybe.fromCallable(new Callable<Integer>() {
                @Override
                public Integer call() throws Exception {
                    return count.incrementAndGet() - j;
                }
            })
            .subscribeOn(Schedulers.io());
        }
        sources[1] = Maybe.fromCallable(new Callable<Integer>() {
            @Override
            public Integer call() throws Exception {
                throw new TestException("" + count.incrementAndGet());
            }
        })
        .subscribeOn(Schedulers.io());

        for (int i = 0; i < 1000; i++) {
            count.set(0);
            Maybe.mergeDelayError(
                    Flowable.fromArray(sources), 1)
            .test()
            .awaitDone(5, TimeUnit.SECONDS)
            .assertFailureAndMessage(TestException.class, "2", 0, 0);
        }
    }

    @Test
    public void scalar() {
        Maybe.mergeDelayError(
                Flowable.just(Maybe.just(1)))
        .test()
        .assertResult(1);
    }
}
