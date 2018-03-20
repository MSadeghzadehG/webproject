

package io.reactivex.maybe;

import io.reactivex.Maybe;
import io.reactivex.functions.Predicate;
import io.reactivex.internal.functions.Functions;
import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicInteger;
import org.junit.Test;

import static org.junit.Assert.assertEquals;

public class MaybeRetryTest {
    @Test
    public void retryTimesPredicateWithMatchingPredicate() {
        final AtomicInteger atomicInteger = new AtomicInteger(3);
        final AtomicInteger numberOfSubscribeCalls = new AtomicInteger(0);

        Maybe.fromCallable(new Callable<Boolean>() {
            @Override public Boolean call() throws Exception {
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

        Maybe.fromCallable(new Callable<Boolean>() {
            @Override public Boolean call() throws Exception {
                numberOfSubscribeCalls.incrementAndGet();

                if (atomicInteger.decrementAndGet() != 0) {
                    throw new RuntimeException();
                }

                return true;
            }
        })
            .retry(2, Functions.alwaysTrue())
            .test()
            .assertResult(true);

        assertEquals(3, numberOfSubscribeCalls.get());
    }

    @Test
    public void retryTimesPredicateWithNotMatchingRetryAmount() {
        final AtomicInteger atomicInteger = new AtomicInteger(3);
        final AtomicInteger numberOfSubscribeCalls = new AtomicInteger(0);

        Maybe.fromCallable(new Callable<Boolean>() {
            @Override public Boolean call() throws Exception {
                numberOfSubscribeCalls.incrementAndGet();

                if (atomicInteger.decrementAndGet() != 0) {
                    throw new RuntimeException();
                }

                return true;
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

        Maybe.fromCallable(new Callable<Boolean>() {
            @Override public Boolean call() throws Exception {
                numberOfSubscribeCalls.incrementAndGet();

                if (atomicInteger.decrementAndGet() != 0) {
                    throw new RuntimeException();
                }

                return true;
            }
        })
            .retry(0, Functions.alwaysTrue())
            .test()
            .assertFailure(RuntimeException.class);

        assertEquals(1, numberOfSubscribeCalls.get());
    }
}
