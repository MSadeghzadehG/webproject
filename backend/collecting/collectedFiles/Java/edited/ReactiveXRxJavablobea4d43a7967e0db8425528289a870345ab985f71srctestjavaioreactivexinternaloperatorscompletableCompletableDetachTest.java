

package io.reactivex.internal.operators.completable;

import static org.junit.Assert.assertNull;

import java.io.IOException;
import java.lang.ref.WeakReference;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.disposables.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.Function;
import io.reactivex.observers.TestObserver;
import io.reactivex.processors.PublishProcessor;

public class CompletableDetachTest {

    @Test
    public void doubleSubscribe() {

        TestHelper.checkDoubleOnSubscribeCompletable(new Function<Completable, CompletableSource>() {
            @Override
            public CompletableSource apply(Completable m) throws Exception {
                return m.onTerminateDetach();
            }
        });
    }

    @Test
    public void dispose() {
        TestHelper.checkDisposed(PublishProcessor.create().ignoreElements().onTerminateDetach());
    }

    @Test
    public void onError() {
        Completable.error(new TestException())
        .onTerminateDetach()
        .test()
        .assertFailure(TestException.class);
    }

    @Test
    public void onComplete() {
        Completable.complete()
        .onTerminateDetach()
        .test()
        .assertResult();
    }

    @Test
    public void cancelDetaches() throws Exception {
        Disposable d = Disposables.empty();
        final WeakReference<Disposable> wr = new WeakReference<Disposable>(d);

        TestObserver<Void> to = new Completable() {
            @Override
            protected void subscribeActual(CompletableObserver observer) {
                observer.onSubscribe(wr.get());
            };
        }
        .onTerminateDetach()
        .test();

        d = null;

        to.cancel();

        System.gc();
        Thread.sleep(200);

        to.assertEmpty();

        assertNull(wr.get());
    }

    @Test
    public void completeDetaches() throws Exception {
        Disposable d = Disposables.empty();
        final WeakReference<Disposable> wr = new WeakReference<Disposable>(d);

        TestObserver<Void> to = new Completable() {
            @Override
            protected void subscribeActual(CompletableObserver observer) {
                observer.onSubscribe(wr.get());
                observer.onComplete();
                observer.onComplete();
            };
        }
        .onTerminateDetach()
        .test();

        d = null;

        System.gc();
        Thread.sleep(200);

        to.assertResult();

        assertNull(wr.get());
    }

    @Test
    public void errorDetaches() throws Exception {
        Disposable d = Disposables.empty();
        final WeakReference<Disposable> wr = new WeakReference<Disposable>(d);

        TestObserver<Void> to = new Completable() {
            @Override
            protected void subscribeActual(CompletableObserver observer) {
                observer.onSubscribe(wr.get());
                observer.onError(new TestException());
                observer.onError(new IOException());
            };
        }
        .onTerminateDetach()
        .test();

        d = null;

        System.gc();
        Thread.sleep(200);

        to.assertFailure(TestException.class);

        assertNull(wr.get());
    }
}
