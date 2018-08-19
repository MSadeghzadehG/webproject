

package io.reactivex.completable;

import io.reactivex.Completable;
import io.reactivex.functions.Action;
import io.reactivex.functions.Predicate;
import io.reactivex.internal.functions.Functions;
import java.util.concurrent.atomic.AtomicInteger;
import org.junit.Test;

import static org.junit.Assert.assertEquals;

public class CompletableRetryTest {
    @Test
    public void retryTimesPredicateWithMatchingPredicate() {
        final AtomicInteger atomicInteger = new AtomicInteger(3);
        final AtomicInteger numberOfSubscribeCalls = new AtomicInteger(0);

        Completable.fromAction(new Action() {
            @Override public void run() throws Exception {
                numberOfSubscribeCalls.incrementAndGet();

                if (atomicInteger.decrementAndGet() != 0) {
                    throw new RuntimeException();
                }

                throw new IllegalArgumentException();
            }
        })
            .retry(Integer.MAX_VALUE, new Predicate<Throwable>() {
                @Override public boolean test(final Throwable throwable) throws Exception {
                    return !(throwable instanceof IllegalArgumentException);
                }
            })
            .test()
            .assertFailure(IllegalArgumentException.class);

        assertEquals(3, numberOfSubscribeCalls.get());
    }

    @Test
    public void retryTimesPredicateWithMatchingRetryAmount() {
        final AtomicInteger atomicInteger = new AtomicInteger(3);
        final AtomicInteger numberOfSubscribeCalls = new AtomicInteger(0);

        Completable.fromAction(new Action() {
            @Override public void run() throws Exception {
                numberOfSubscribeCalls.incrementAndGet();

                if (atomicInteger.decrementAndGet() != 0) {
                    throw new RuntimeException();
                }
            }
        })
            .retry(2, Functions.alwaysTrue())
            .test()
            .assertResult();

        assertEquals(3, numberOfSubscribeCalls.get());
    }

    @Test
    public void retryTimesPredicateWithNotMatchingRetryAmount() {
        final AtomicInteger atomicInteger = new AtomicInteger(3);
        final AtomicInteger numberOfSubscribeCalls = new AtomicInteger(0);

        Completable.fromAction(new Action() {
            @Override public void run() throws Exception {
                numberOfSubscribeCalls.incrementAndGet();

                if (atomicInteger.decrementAndGet() != 0) {
                    throw new RuntimeException();
                }
            }
        })
            .retry(1, Functions.alwaysTrue())
            .test()
            .assertFailure(RuntimeException.class);

        assertEquals(2, numberOfSubscribeCalls.get());
    }

    @Test
    public void retryTimesPredicateWithZeroRetries() {
        final AtomicInteger atomicInteger = new AtomicInteger(2);
        final AtomicInteger numberOfSubscribeCalls = new AtomicInteger(0);

        Completable.fromAction(new Action() {
            @Override public void run() throws Exception {
                numberOfSubscribeCalls.incrementAndGet();

                if (atomicInteger.decrementAndGet() != 0) {
                    throw new RuntimeException();
                }
            }
        })
            .retry(0, Functions.alwaysTrue())
            .test()
            .assertFailure(RuntimeException.class);

        assertEquals(1, numberOfSubscribeCalls.get());
    }
}
