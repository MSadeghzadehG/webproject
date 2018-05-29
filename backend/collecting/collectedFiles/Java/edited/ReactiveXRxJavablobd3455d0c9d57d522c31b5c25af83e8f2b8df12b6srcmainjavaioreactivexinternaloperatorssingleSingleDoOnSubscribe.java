

package io.reactivex.internal.operators.single;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Consumer;
import io.reactivex.internal.disposables.EmptyDisposable;
import io.reactivex.plugins.RxJavaPlugins;


public final class SingleDoOnSubscribe<T> extends Single<T> {

    final SingleSource<T> source;

    final Consumer<? super Disposable> onSubscribe;

    public SingleDoOnSubscribe(SingleSource<T> source, Consumer<? super Disposable> onSubscribe) {
        this.source = source;
        this.onSubscribe = onSubscribe;
    }

    @Override
    protected void subscribeActual(final SingleObserver<? super T> s) {
        source.subscribe(new DoOnSubscribeSingleObserver<T>(s, onSubscribe));
    }

    static final class DoOnSubscribeSingleObserver<T> implements SingleObserver<T> {

        final SingleObserver<? super T> actual;

        final Consumer<? super Disposable> onSubscribe;

        boolean done;

        DoOnSubscribeSingleObserver(SingleObserver<? super T> actual, Consumer<? super Disposable> onSubscribe) {
            this.actual = actual;
            this.onSubscribe = onSubscribe;
        }

        @Override
        public void onSubscribe(Disposable d) {
            try {
                onSubscribe.accept(d);
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                done = true;
                d.dispose();
                EmptyDisposable.error(ex, actual);
                return;
            }

            actual.onSubscribe(d);
        }

        @Override
        public void onSuccess(T value) {
            if (done) {
                return;
            }
            actual.onSuccess(value);
        }

        @Override
        public void onError(Throwable e) {
            if (done) {
                RxJavaPlugins.onError(e);
                return;
            }
            actual.onError(e);
        }
    }

}
