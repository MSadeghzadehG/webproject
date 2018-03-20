

package io.reactivex.internal.util;

import static org.junit.Assert.*;

import java.util.concurrent.*;

import org.junit.Test;

import io.reactivex.TestHelper;
import io.reactivex.disposables.*;
import io.reactivex.schedulers.Schedulers;

public class BlockingHelperTest {

    @Test
    public void emptyEnum() {
        TestHelper.checkUtilityClass(BlockingHelper.class);
    }

    @Test
    public void interrupted() {
        CountDownLatch cdl = new CountDownLatch(1);
        Disposable d = Disposables.empty();

        Thread.currentThread().interrupt();

        try {
            BlockingHelper.awaitForComplete(cdl, d);
        } catch (IllegalStateException ex) {
                    }
        assertTrue(d.isDisposed());
        assertTrue(Thread.interrupted());
    }

    @Test
    public void unblock() {
        final CountDownLatch cdl = new CountDownLatch(1);
        Disposable d = Disposables.empty();

        Schedulers.computation().scheduleDirect(new Runnable() {
            @Override
            public void run() {
                cdl.countDown();
            }
        }, 100, TimeUnit.MILLISECONDS);

        BlockingHelper.awaitForComplete(cdl, d);

        assertFalse(d.isDisposed());
    }
}
