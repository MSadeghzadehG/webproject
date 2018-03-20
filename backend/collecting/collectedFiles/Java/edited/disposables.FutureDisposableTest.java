

package io.reactivex.disposables;

import java.util.concurrent.FutureTask;

import static org.junit.Assert.*;
import org.junit.Test;

import io.reactivex.internal.functions.Functions;

public class FutureDisposableTest {

    @Test
    public void normal() {
        FutureTask<Object> ft = new FutureTask<Object>(Functions.EMPTY_RUNNABLE, null);
        Disposable d = Disposables.fromFuture(ft);
        assertFalse(d.isDisposed());

        d.dispose();

        assertTrue(d.isDisposed());

        d.dispose();

        assertTrue(d.isDisposed());

        assertTrue(ft.isCancelled());
    }

    @Test
    public void interruptible() {
        FutureTask<Object> ft = new FutureTask<Object>(Functions.EMPTY_RUNNABLE, null);
        Disposable d = Disposables.fromFuture(ft, true);
        assertFalse(d.isDisposed());

        d.dispose();

        assertTrue(d.isDisposed());

        d.dispose();

        assertTrue(d.isDisposed());

        assertTrue(ft.isCancelled());
    }

    @Test
    public void normalDone() {
        FutureTask<Object> ft = new FutureTask<Object>(Functions.EMPTY_RUNNABLE, null);
        FutureDisposable d = new FutureDisposable(ft, false);
        assertFalse(d.isDisposed());

        assertFalse(d.isDisposed());

        ft.run();

        assertTrue(d.isDisposed());
    }
}
