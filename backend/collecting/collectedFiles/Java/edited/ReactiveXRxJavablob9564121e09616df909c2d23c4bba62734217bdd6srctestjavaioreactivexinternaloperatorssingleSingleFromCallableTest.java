

package io.reactivex.internal.operators.single;

import io.reactivex.Observer;
import io.reactivex.Single;
import io.reactivex.SingleObserver;
import io.reactivex.TestHelper;
import io.reactivex.disposables.Disposable;
import io.reactivex.observers.TestObserver;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.schedulers.Schedulers;
import org.junit.Test;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;

import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.*;

public class SingleFromCallableTest {

    @Test
    public void fromCallableValue() {
        Single.fromCallable(new Callable<Integer>() {
            @Override public Integer call() throws Exception {
                return 5;
            }
        })
            .test()
            .assertResult(5);
    }

    @Test
    public void fromCallableError() {
        Single.fromCallable(new Callable<Integer>() {
            @Override public Integer call() throws Exception {
                throw new UnsupportedOperationException();
            }
        })
            .test()
            .assertFailure(UnsupportedOperationException.class);
    }

    @Test
    public void fromCallableNull() {
        Single.fromCallable(new Callable<Integer>() {
            @Override public Integer call() throws Exception {
                return null;
            }
        })
            .test()
            .assertFailureAndMessage(NullPointerException.class, "The callable returned a null value");
    }

    @Test
    public void fromCallableTwice() {
        final AtomicInteger atomicInteger = new AtomicInteger();

        Callable<Integer> callable = new Callable<Integer>() {
            @Override
            public Integer call() throws Exception {
                return atomicInteger.incrementAndGet();
            }
        };

        Single.fromCallable(callable)
                .test()
                .assertResult(1);

        assertEquals(1, atomicInteger.get());

        Single.fromCallable(callable)
                .test()
                .assertResult(2);

        assertEquals(2, atomicInteger.get());
    }

    @SuppressWarnings("unchecked")
    @Test
    public void shouldNotInvokeFuncUntilSubscription() throws Exception {
        Callable<Object> func = mock(Callable.class);

        when(func.call()).thenReturn(new Object());

        Single<Object> fromCallableSingle = Single.fromCallable(func);

        verifyZeroInteractions(func);

        fromCallableSingle.subscribe();

        verify(func).call();
    }


    @Test
    public void noErrorLoss() throws Exception {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            final CountDownLatch cdl1 = new CountDownLatch(1);
            final CountDownLatch cdl2 = new CountDownLatch(1);

            TestObserver<Integer> to = Single.fromCallable(new Callable<Integer>() {
                @Override
                public Integer call() throws Exception {
                    cdl1.countDown();
                    cdl2.await(5, TimeUnit.SECONDS);
                    return 1;
                }
            }).subscribeOn(Schedulers.single()).test();

            assertTrue(cdl1.await(5, TimeUnit.SECONDS));

            to.cancel();

            int timeout = 10;

            while (timeout-- > 0 && errors.isEmpty()) {
                Thread.sleep(100);
            }

            TestHelper.assertUndeliverable(errors, 0, InterruptedException.class);
        } finally {
            RxJavaPlugins.reset();
        }
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

        Single<String> fromCallableObservable = Single.fromCallable(func);

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
    public void shouldAllowToThrowCheckedException() {
        final Exception checkedException = new Exception("test exception");

        Single<Object> fromCallableObservable = Single.fromCallable(new Callable<Object>() {
            @Override
            public Object call() throws Exception {
                throw checkedException;
            }
        });

        SingleObserver<Object> observer = TestHelper.mockSingleObserver();

        fromCallableObservable.subscribe(observer);

        verify(observer).onSubscribe(any(Disposable.class));
        verify(observer).onError(checkedException);
        verifyNoMoreInteractions(observer);
    }

    @Test
    public void disposedOnArrival() {
        final int[] count = { 0 };
        Single.fromCallable(new Callable<Object>() {
            @Override
            public Object call() throws Exception {
                count[0]++;
                return 1;
            }
        })
                .test(true)
                .assertEmpty();

        assertEquals(0, count[0]);
    }

    @Test
    public void disposedOnCall() {
        final TestObserver<Integer> to = new TestObserver<Integer>();

        Single.fromCallable(new Callable<Integer>() {
            @Override
            public Integer call() throws Exception {
                to.cancel();
                return 1;
            }
        })
                .subscribe(to);

        to.assertEmpty();
    }

    @Test
    public void toObservableTake() {
        Single.fromCallable(new Callable<Object>() {
            @Override
            public Object call() throws Exception {
                return 1;
            }
        })
                .toObservable()
                .take(1)
                .test()
                .assertResult(1);
    }

    @Test
    public void toObservableAndBack() {
        Single.fromCallable(new Callable<Integer>() {
            @Override
            public Integer call() throws Exception {
                return 1;
            }
        })
                .toObservable()
                .singleOrError()
                .test()
                .assertResult(1);
    }
}
