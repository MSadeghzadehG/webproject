

package io.reactivex.internal.operators.observable;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.*;
import io.reactivex.plugins.RxJavaPlugins;

public final class ObservableTake<T> extends AbstractObservableWithUpstream<T, T> {
    final long limit;
    public ObservableTake(ObservableSource<T> source, long limit) {
        super(source);
        this.limit = limit;
    }

    @Override
    protected void subscribeActual(Observer<? super T> observer) {
        source.subscribe(new TakeObserver<T>(observer, limit));
    }

    static final class TakeObserver<T> implements Observer<T>, Disposable {
        final Observer<? super T> actual;

        boolean done;

        Disposable subscription;

        long remaining;
        TakeObserver(Observer<? super T> actual, long limit) {
            this.actual = actual;
            this.remaining = limit;
        }
        @Override
        public void onSubscribe(Disposable s) {
            if (DisposableHelper.validate(this.subscription, s)) {
                subscription = s;
                if (remaining == 0) {
                    done = true;
                    s.dispose();
                    EmptyDisposable.complete(actual);
                } else {
                    actual.onSubscribe(this);
                }
            }
        }
        @Override
        public void onNext(T t) {
            if (!done && remaining-- > 0) {
                boolean stop = remaining == 0;
                actual.onNext(t);
                if (stop) {
                    onComplete();
                }
            }
        }
        @Override
        public void onError(Throwable t) {
            if (done) {
                RxJavaPlugins.onError(t);
                return;
            }

            done = true;
            subscription.dispose();
            actual.onError(t);
        }
        @Override
        public void onComplete() {
            if (!done) {
                done = true;
                subscription.dispose();
                actual.onComplete();
            }
        }

        @Override
        public void dispose() {
            subscription.dispose();
        }

        @Override
        public boolean isDisposed() {
            return subscription.isDisposed();
        }
    }
}
