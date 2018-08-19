
package io.reactivex.observers;

import io.reactivex.Observer;
import io.reactivex.annotations.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.*;
import io.reactivex.internal.disposables.*;
import io.reactivex.plugins.RxJavaPlugins;


public final class SafeObserver<T> implements Observer<T>, Disposable {
    
    final Observer<? super T> actual;
    
    Disposable s;
    
    boolean done;

    
    public SafeObserver(@NonNull Observer<? super T> actual) {
        this.actual = actual;
    }

    @Override
    public void onSubscribe(@NonNull Disposable s) {
        if (DisposableHelper.validate(this.s, s)) {
            this.s = s;
            try {
                actual.onSubscribe(this);
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                done = true;
                                try {
                    s.dispose();
                } catch (Throwable e1) {
                    Exceptions.throwIfFatal(e1);
                    RxJavaPlugins.onError(new CompositeException(e, e1));
                    return;
                }
                RxJavaPlugins.onError(e);
            }
        }
    }


    @Override
    public void dispose() {
        s.dispose();
    }

    @Override
    public boolean isDisposed() {
        return s.isDisposed();
    }

    @Override
    public void onNext(@NonNull T t) {
        if (done) {
            return;
        }
        if (s == null) {
            onNextNoSubscription();
            return;
        }

        if (t == null) {
            Throwable ex = new NullPointerException("onNext called with null. Null values are generally not allowed in 2.x operators and sources.");
            try {
                s.dispose();
            } catch (Throwable e1) {
                Exceptions.throwIfFatal(e1);
                onError(new CompositeException(ex, e1));
                return;
            }
            onError(ex);
            return;
        }

        try {
            actual.onNext(t);
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            try {
                s.dispose();
            } catch (Throwable e1) {
                Exceptions.throwIfFatal(e1);
                onError(new CompositeException(e, e1));
                return;
            }
            onError(e);
        }
    }

    void onNextNoSubscription() {
        done = true;

        Throwable ex = new NullPointerException("Subscription not set!");

        try {
            actual.onSubscribe(EmptyDisposable.INSTANCE);
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
                        RxJavaPlugins.onError(new CompositeException(ex, e));
            return;
        }
        try {
            actual.onError(ex);
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
                        RxJavaPlugins.onError(new CompositeException(ex, e));
        }
    }

    @Override
    public void onError(@NonNull Throwable t) {
        if (done) {
            RxJavaPlugins.onError(t);
            return;
        }
        done = true;

        if (s == null) {
            Throwable npe = new NullPointerException("Subscription not set!");

            try {
                actual.onSubscribe(EmptyDisposable.INSTANCE);
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                                RxJavaPlugins.onError(new CompositeException(t, npe, e));
                return;
            }
            try {
                actual.onError(new CompositeException(t, npe));
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                                RxJavaPlugins.onError(new CompositeException(t, npe, e));
            }
            return;
        }

        if (t == null) {
            t = new NullPointerException("onError called with null. Null values are generally not allowed in 2.x operators and sources.");
        }

        try {
            actual.onError(t);
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);

            RxJavaPlugins.onError(new CompositeException(t, ex));
        }
    }

    @Override
    public void onComplete() {
        if (done) {
            return;
        }

        done = true;

        if (s == null) {
            onCompleteNoSubscription();
            return;
        }

        try {
            actual.onComplete();
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            RxJavaPlugins.onError(e);
        }
    }

    void onCompleteNoSubscription() {

        Throwable ex = new NullPointerException("Subscription not set!");

        try {
            actual.onSubscribe(EmptyDisposable.INSTANCE);
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
                        RxJavaPlugins.onError(new CompositeException(ex, e));
            return;
        }
        try {
            actual.onError(ex);
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
                        RxJavaPlugins.onError(new CompositeException(ex, e));
        }
    }

}
