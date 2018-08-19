

package io.reactivex.internal.operators.maybe;

import io.reactivex.*;
import io.reactivex.annotations.Experimental;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Consumer;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.plugins.RxJavaPlugins;


@Experimental
public final class MaybeDoAfterSuccess<T> extends AbstractMaybeWithUpstream<T, T> {

    final Consumer<? super T> onAfterSuccess;

    public MaybeDoAfterSuccess(MaybeSource<T> source, Consumer<? super T> onAfterSuccess) {
        super(source);
        this.onAfterSuccess = onAfterSuccess;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> s) {
        source.subscribe(new DoAfterObserver<T>(s, onAfterSuccess));
    }

    static final class DoAfterObserver<T> implements MaybeObserver<T>, Disposable {

        final MaybeObserver<? super T> actual;

        final Consumer<? super T> onAfterSuccess;

        Disposable d;

        DoAfterObserver(MaybeObserver<? super T> actual, Consumer<? super T> onAfterSuccess) {
            this.actual = actual;
            this.onAfterSuccess = onAfterSuccess;
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                this.d = d;

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onSuccess(T t) {
            actual.onSuccess(t);

            try {
                onAfterSuccess.accept(t);
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                             RxJavaPlugins.onError(ex);
            }
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            actual.onComplete();
        }

        @Override
        public void dispose() {
            d.dispose();
        }

        @Override
        public boolean isDisposed() {
            return d.isDisposed();
        }
    }
}
