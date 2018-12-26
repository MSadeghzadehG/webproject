
package io.reactivex.subscribers;

import org.reactivestreams.*;

import io.reactivex.FlowableSubscriber;
import io.reactivex.exceptions.*;
import io.reactivex.internal.subscriptions.*;
import io.reactivex.plugins.RxJavaPlugins;


public final class SafeSubscriber<T> implements FlowableSubscriber<T>, Subscription {
    
    final Subscriber<? super T> actual;
    
    Subscription s;
    
    boolean done;

    
    public SafeSubscriber(Subscriber<? super T> actual) {
        this.actual = actual;
    }

    @Override
    public void onSubscribe(Subscription s) {
        if (SubscriptionHelper.validate(this.s, s)) {
            this.s = s;
            try {
                actual.onSubscribe(this);
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                done = true;
                                try {
                    s.cancel();
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
    public void onNext(T t) {
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
                s.cancel();
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
                s.cancel();
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
            actual.onSubscribe(EmptySubscription.INSTANCE);
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
    public void onError(Throwable t) {
        if (done) {
            RxJavaPlugins.onError(t);
            return;
        }
        done = true;

        if (s == null) {
            Throwable npe = new NullPointerException("Subscription not set!");

            try {
                actual.onSubscribe(EmptySubscription.INSTANCE);
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
            actual.onSubscribe(EmptySubscription.INSTANCE);
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
    public void request(long n) {
        try {
            s.request(n);
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            try {
                s.cancel();
            } catch (Throwable e1) {
                Exceptions.throwIfFatal(e1);
                RxJavaPlugins.onError(new CompositeException(e, e1));
                return;
            }
            RxJavaPlugins.onError(e);
        }
    }

    @Override
    public void cancel() {
        try {
            s.cancel();
        } catch (Throwable e1) {
            Exceptions.throwIfFatal(e1);
            RxJavaPlugins.onError(e1);
        }
    }
}
