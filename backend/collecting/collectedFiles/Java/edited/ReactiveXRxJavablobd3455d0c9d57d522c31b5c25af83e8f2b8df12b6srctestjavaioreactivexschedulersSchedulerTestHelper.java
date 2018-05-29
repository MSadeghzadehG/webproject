

package io.reactivex.schedulers;

import static org.junit.Assert.*;

import java.util.concurrent.*;

import io.reactivex.*;
import io.reactivex.subscribers.DefaultSubscriber;

final class SchedulerTestHelper {
    private SchedulerTestHelper() {
            }

    
    static void testUnhandledErrorIsDeliveredToThreadHandler(Scheduler scheduler) throws InterruptedException {
        Thread.UncaughtExceptionHandler originalHandler = Thread.getDefaultUncaughtExceptionHandler();
        try {
            CapturingUncaughtExceptionHandler handler = new CapturingUncaughtExceptionHandler();
            Thread.setDefaultUncaughtExceptionHandler(handler);
            IllegalStateException error = new IllegalStateException("Should be delivered to handler");
            Flowable.error(error)
                    .subscribeOn(scheduler)
                    .subscribe();

            if (!handler.completed.await(3, TimeUnit.SECONDS)) {
                fail("timed out");
            }

            assertEquals("Should have received exactly 1 exception", 1, handler.count);
            Throwable cause = handler.caught;
            while (cause != null) {
                if (error.equals(cause)) { break; }
                if (cause == cause.getCause()) { break; }
                cause = cause.getCause();
            }
            assertEquals("Our error should have been delivered to the handler", error, cause);
        } finally {
            Thread.setDefaultUncaughtExceptionHandler(originalHandler);
        }
    }

    
    static void testHandledErrorIsNotDeliveredToThreadHandler(Scheduler scheduler) throws InterruptedException {
        Thread.UncaughtExceptionHandler originalHandler = Thread.getDefaultUncaughtExceptionHandler();
        try {
            CapturingUncaughtExceptionHandler handler = new CapturingUncaughtExceptionHandler();
            CapturingObserver<Object> observer = new CapturingObserver<Object>();
            Thread.setDefaultUncaughtExceptionHandler(handler);
            IllegalStateException error = new IllegalStateException("Should be delivered to handler");
            Flowable.error(error)
                    .subscribeOn(scheduler)
                    .subscribe(observer);

            if (!observer.completed.await(3, TimeUnit.SECONDS)) {
                fail("timed out");
            }

            if (handler.count != 0) {
                handler.caught.printStackTrace();
            }
            assertEquals("Handler should not have received anything: " + handler.caught, 0, handler.count);
            assertEquals("Observer should have received an error", 1, observer.errorCount);
            assertEquals("Observer should not have received a next value", 0, observer.nextCount);

            Throwable cause = observer.error;
            while (cause != null) {
                if (error.equals(cause)) { break; }
                if (cause == cause.getCause()) { break; }
                cause = cause.getCause();
            }
            assertEquals("Our error should have been delivered to the observer", error, cause);
        } finally {
            Thread.setDefaultUncaughtExceptionHandler(originalHandler);
        }
    }

    private static final class CapturingUncaughtExceptionHandler implements Thread.UncaughtExceptionHandler {
        int count;
        Throwable caught;
        CountDownLatch completed = new CountDownLatch(1);

        @Override
        public void uncaughtException(Thread t, Throwable e) {
            count++;
            caught = e;
            completed.countDown();
        }
    }

    static final class CapturingObserver<T> extends DefaultSubscriber<T> {
        CountDownLatch completed = new CountDownLatch(1);
        int errorCount;
        int nextCount;
        Throwable error;

        @Override
        public void onComplete() {
        }

        @Override
        public void onError(Throwable e) {
            errorCount++;
            error = e;
            completed.countDown();
        }

        @Override
        public void onNext(T t) {
            nextCount++;
        }
    }
}
