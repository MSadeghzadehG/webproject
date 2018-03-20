

package io.reactivex.internal.util;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.plugins.RxJavaPlugins;


public enum EmptyComponent implements FlowableSubscriber<Object>, Observer<Object>, MaybeObserver<Object>,
SingleObserver<Object>, CompletableObserver, Subscription, Disposable {
    INSTANCE;

    @SuppressWarnings("unchecked")
    public static <T> Subscriber<T> asSubscriber() {
        return (Subscriber<T>)INSTANCE;
    }

    @SuppressWarnings("unchecked")
    public static <T> Observer<T> asObserver() {
        return (Observer<T>)INSTANCE;
    }

    @Override
    public void dispose() {
            }

    @Override
    public boolean isDisposed() {
        return true;
    }

    @Override
    public void request(long n) {
            }

    @Override
    public void cancel() {
            }

    @Override
    public void onSubscribe(Disposable d) {
        d.dispose();
    }

    @Override
    public void onSubscribe(Subscription s) {
        s.cancel();
    }

    @Override
    public void onNext(Object t) {
            }

    @Override
    public void onError(Throwable t) {
        RxJavaPlugins.onError(t);
    }

    @Override
    public void onComplete() {
            }

    @Override
    public void onSuccess(Object value) {
            }
}
