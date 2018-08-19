
package io.reactivex.internal.operators.flowable;

import static org.junit.Assert.*;

import java.util.*;
import java.util.concurrent.*;

import org.junit.*;
import org.reactivestreams.*;

import io.reactivex.Flowable;
import io.reactivex.exceptions.TestException;
import io.reactivex.internal.subscriptions.BooleanSubscription;

public class BlockingFlowableToFutureTest {
    @Ignore("No separate file")
    @Test
    public void constructorShouldBePrivate() {
    }

    @Test
    public void testToFuture() throws InterruptedException, ExecutionException {
        Flowable<String> obs = Flowable.just("one");
        Future<String> f = obs.toFuture();
        assertEquals("one", f.get());
    }

    @Test
    public void testToFutureList() throws InterruptedException, ExecutionException {
        Flowable<String> obs = Flowable.just("one", "two", "three");
        Future<List<String>> f = obs.toList().toFuture();
        assertEquals("one", f.get().get(0));
        assertEquals("two", f.get().get(1));
        assertEquals("three", f.get().get(2));
    }

    @Test(expected = IndexOutOfBoundsException.class)
    public void testExceptionWithMoreThanOneElement() throws Throwable {
        Flowable<String> obs = Flowable.just("one", "two");
        Future<String> f = obs.toFuture();
        try {
                        f.get();
            fail("Should have thrown!");
        }
        catch (ExecutionException e) {
            throw e.getCause();
        }
    }

    @Test
    public void testToFutureWithException() {
        Flowable<String> obs = Flowable.unsafeCreate(new Publisher<String>() {

            @Override
            public void subscribe(Subscriber<? super String> observer) {
                observer.onSubscribe(new BooleanSubscription());
                observer.onNext("one");
                observer.onError(new TestException());
            }
        });

        Future<String> f = obs.toFuture();
        try {
            f.get();
            fail("expected exception");
        } catch (Throwable e) {
            assertEquals(TestException.class, e.getCause().getClass());
        }
    }

    @Test(expected = CancellationException.class)
    public void testGetAfterCancel() throws Exception {
        Flowable<String> obs = Flowable.never();
        Future<String> f = obs.toFuture();
        boolean cancelled = f.cancel(true);
        assertTrue(cancelled);          f.get();                    }

    @Test(expected = CancellationException.class)
    public void testGetWithTimeoutAfterCancel() throws Exception {
        Flowable<String> obs = Flowable.never();
        Future<String> f = obs.toFuture();
        boolean cancelled = f.cancel(true);
        assertTrue(cancelled);          f.get(Long.MAX_VALUE, TimeUnit.NANOSECONDS);        }

    @Test(expected = NoSuchElementException.class)
    public void testGetWithEmptyFlowable() throws Throwable {
        Flowable<String> obs = Flowable.empty();
        Future<String> f = obs.toFuture();
        try {
            f.get();
        }
        catch (ExecutionException e) {
            throw e.getCause();
        }
    }

    @Ignore("null value is not allowed")
    @Test
    public void testGetWithASingleNullItem() throws Exception {
        Flowable<String> obs = Flowable.just((String)null);
        Future<String> f = obs.toFuture();
        assertEquals(null, f.get());
    }
}
