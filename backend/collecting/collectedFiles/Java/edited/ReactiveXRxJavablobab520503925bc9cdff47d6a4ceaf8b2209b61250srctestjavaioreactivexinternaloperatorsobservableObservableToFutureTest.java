

package io.reactivex.internal.operators.observable;

import static org.mockito.Mockito.*;

import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.observers.TestObserver;
import io.reactivex.schedulers.*;

public class ObservableToFutureTest {

    @Test
    public void testSuccess() throws Exception {
        @SuppressWarnings("unchecked")
        Future<Object> future = mock(Future.class);
        Object value = new Object();
        when(future.get()).thenReturn(value);

        Observer<Object> o = TestHelper.mockObserver();

        TestObserver<Object> to = new TestObserver<Object>(o);

        Observable.fromFuture(future).subscribe(to);

        to.dispose();

        verify(o, times(1)).onNext(value);
        verify(o, times(1)).onComplete();
        verify(o, never()).onError(any(Throwable.class));
        verify(future, never()).cancel(true);
    }

    @Test
    public void testSuccessOperatesOnSuppliedScheduler() throws Exception {
        @SuppressWarnings("unchecked")
        Future<Object> future = mock(Future.class);
        Object value = new Object();
        when(future.get()).thenReturn(value);

        Observer<Object> o = TestHelper.mockObserver();

        TestScheduler scheduler = new TestScheduler();
        TestObserver<Object> to = new TestObserver<Object>(o);

        Observable.fromFuture(future, scheduler).subscribe(to);

        verify(o, never()).onNext(value);

        scheduler.triggerActions();

        verify(o, times(1)).onNext(value);
    }

    @Test
    public void testFailure() throws Exception {
        @SuppressWarnings("unchecked")
        Future<Object> future = mock(Future.class);
        RuntimeException e = new RuntimeException();
        when(future.get()).thenThrow(e);

        Observer<Object> o = TestHelper.mockObserver();

        TestObserver<Object> to = new TestObserver<Object>(o);

        Observable.fromFuture(future).subscribe(to);

        to.dispose();

        verify(o, never()).onNext(null);
        verify(o, never()).onComplete();
        verify(o, times(1)).onError(e);
        verify(future, never()).cancel(true);
    }

    @Test
    public void testCancelledBeforeSubscribe() throws Exception {
        @SuppressWarnings("unchecked")
        Future<Object> future = mock(Future.class);
        CancellationException e = new CancellationException("unit test synthetic cancellation");
        when(future.get()).thenThrow(e);

        Observer<Object> o = TestHelper.mockObserver();

        TestObserver<Object> to = new TestObserver<Object>(o);
        to.dispose();

        Observable.fromFuture(future).subscribe(to);

        to.assertNoErrors();
        to.assertNotComplete();
    }

    @Test
    public void testCancellationDuringFutureGet() throws Exception {
        Future<Object> future = new Future<Object>() {
            private AtomicBoolean isCancelled = new AtomicBoolean(false);
            private AtomicBoolean isDone = new AtomicBoolean(false);

            @Override
            public boolean cancel(boolean mayInterruptIfRunning) {
                isCancelled.compareAndSet(false, true);
                return true;
            }

            @Override
            public boolean isCancelled() {
                return isCancelled.get();
            }

            @Override
            public boolean isDone() {
                return isCancelled() || isDone.get();
            }

            @Override
            public Object get() throws InterruptedException, ExecutionException {
                Thread.sleep(500);
                isDone.compareAndSet(false, true);
                return "foo";
            }

            @Override
            public Object get(long timeout, TimeUnit unit) throws InterruptedException, ExecutionException, TimeoutException {
                return get();
            }
        };

        Observer<Object> o = TestHelper.mockObserver();

        TestObserver<Object> to = new TestObserver<Object>(o);
        Observable<Object> futureObservable = Observable.fromFuture(future);

        futureObservable.subscribeOn(Schedulers.computation()).subscribe(to);

        Thread.sleep(100);

        to.dispose();

        to.assertNoErrors();
        to.assertNoValues();
        to.assertNotComplete();
    }
}
