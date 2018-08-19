

package io.reactivex.internal.operators.completable;

import static org.junit.Assert.*;

import java.util.concurrent.TimeUnit;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.processors.PublishProcessor;

public class CompletableAwaitTest {

    @Test
    public void awaitInterrupted() {

        Thread.currentThread().interrupt();

        try {
            PublishProcessor.create().ignoreElements().blockingAwait();
            fail("Should have thrown RuntimeException");
        } catch (RuntimeException ex) {
            if (!(ex.getCause() instanceof InterruptedException)) {
                fail("Wrong cause: " + ex.getCause());
            }
        }

    }

    @Test
    public void awaitTimeoutInterrupted() {

        Thread.currentThread().interrupt();

        try {
            PublishProcessor.create().ignoreElements().blockingAwait(1, TimeUnit.SECONDS);
            fail("Should have thrown RuntimeException");
        } catch (RuntimeException ex) {
            if (!(ex.getCause() instanceof InterruptedException)) {
                fail("Wrong cause: " + ex.getCause());
            }
        }

    }

    @Test
    public void awaitTimeout() {
        assertFalse(PublishProcessor.create().ignoreElements().blockingAwait(100, TimeUnit.MILLISECONDS));
    }

    @Test
    public void blockingGet() {
        assertNull(Completable.complete().blockingGet());
    }

    @Test
    public void blockingGetTimeout() {
        assertNull(Completable.complete().blockingGet(1, TimeUnit.SECONDS));
    }

    @Test
    public void blockingGetError() {
        TestException ex = new TestException();
        assertSame(ex, Completable.error(ex).blockingGet());
    }

    @Test
    public void blockingGetErrorTimeout() {
        TestException ex = new TestException();
        assertSame(ex, Completable.error(ex).blockingGet(1, TimeUnit.SECONDS));
    }

}
