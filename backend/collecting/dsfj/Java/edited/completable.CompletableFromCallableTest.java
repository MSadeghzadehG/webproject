

package io.reactivex.internal.operators.completable;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.*;

import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

import org.junit.Test;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.TestException;
import io.reactivex.observers.TestObserver;
import io.reactivex.schedulers.Schedulers;

public class CompletableFromCallableTest {
    @Test(expected = NullPointerException.class)
    public void fromCallableNull() {
        Completable.fromCallable(null);
    }

    @Test
    public void fromCallable() {
        final AtomicInteger atomicInteger = new AtomicInteger();

        Completable.fromCallable(new Callable<Object>() {
            @Override
            public Object call() throws Exception {
                atomicInteger.incrementAndGet();
                return null;
            }
        })
            .test()
            .assertResult();

        assertEquals(1, atomicInteger.get());
    }

    @Test
    public void fromCallableTwice() {
        final AtomicInteger atomicInteger = new AtomicInteger();

        Callable<Object> callable = new Callable<Object>() {
            @Override
            public Object call() throws Exception {
                atomicInteger.incrementAndGet();
                return null;
            }
        };

        Completable.fromCallable(callable)
            .test()
            .assertResult();

        assertEquals(1, atomicInteger.get());

        Completable.fromCallable(callable)
            .test()
            .assertResult();

        assertEquals(2, atomicInteger.get());
    }

    @Test
    public void fromCallableInvokesLazy() {
        final AtomicInteger atomicInteger = new AtomicInteger();

        Completable completable = Completable.fromCallable(new Callable<Object>() {
            @Override
            public Object call() throws Exception {
                atomicInteger.incrementAndGet();
                return null;
            }
        });

        assertEquals(0, atomicInteger.get());

        completable
            .test()
            .assertResult();

        assertEquals(1, atomicInteger.get());
    }

    @Test
    public void fromCallableThrows() {
        Completable.fromCallable(new Callable<Object>() {
            @Override
            public Object call() throws Exception {
                throw new UnsupportedOperationException();
            }
        })
            .test()
            .assertFailure(UnsupportedOperationException.class);
    }

    @SuppressWarnings("unchecked")
    @Test
    public void shouldNotDeliverResultIfSubscriberUnsubscribedBeforeEmission() throws Exception {
        Callable<String> func = mock(Callable.class);

        final CountDownLatch funcLatch = new CountDownLatch(1);
        final CountDownLatch observerLatch = new CountDownLatch(1);

        when(func.call()).thenAnswer(new Answer<String>() {
            @Override
            public String answer(InvocationOnMock invocation) throws Throwable {
                observerLatch.countDown();

                try {
                    funcLatch.await();
                } catch (InterruptedException e) {
                    
                                        Thread.currentThread().interrupt();
                }

                return "should_not_be_delivered";
            }
        });

        Completable fromCallableObservable = Completable.fromCallable(func);

        Observer<Object> observer = TestHelper.mockObserver();

        TestObserver<String> outer = new TestObserver<String>(observer);

        fromCallableObservable
                .subscribeOn(Schedulers.computation())
                .subscribe(outer);

                observerLatch.await();

                outer.cancel();

                funcLatch.countDown();

                verify(func).call();

                verify(observer).onSubscribe(any(Disposable.class));
        verifyNoMoreInteractions(observer);
    }

    @Test
    public void fromActionErrorsDisposed() {
        final AtomicInteger calls = new AtomicInteger();
        Completable.fromCallable(new Callable<Object>() {
            @Override
            public Object call() throws Exception {
                calls.incrementAndGet();
                throw new TestException();
            }
        })
        .test(true)
        .assertEmpty();

        assertEquals(1, calls.get());
    }
}
