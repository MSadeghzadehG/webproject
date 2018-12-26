

package io.reactivex.internal.operators.single;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Action;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.plugins.RxJavaPlugins;

public final class SingleDoOnDispose<T> extends Single<T> {
    final SingleSource<T> source;

    final Action onDispose;

    public SingleDoOnDispose(SingleSource<T> source, Action onDispose) {
        this.source = source;
        this.onDispose = onDispose;
    }

    @Override
    protected void subscribeActual(final SingleObserver<? super T> s) {

        source.subscribe(new DoOnDisposeObserver<T>(s, onDispose));
    }

    static final class DoOnDisposeObserver<T>
    extends AtomicReference<Action>
    implements SingleObserver<T>, Disposable {
        private static final long serialVersionUID = -8583764624474935784L;

        final SingleObserver<? super T> actual;

        Disposable d;

        DoOnDisposeObserver(SingleObserver<? super T> actual, Action onDispose) {
            this.actual = actual;
            this.lazySet(onDispose);
        }

        @Override
        public void dispose() {
            Action a = getAndSet(null);
            if (a != null) {
                try {
                    a.run();
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    RxJavaPlugins.onError(ex);
                }
                d.dispose();
            }
        }

        @Override
        public boolean isDisposed() {
            return d.isDisposed();
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                this.d = d;
                actual.onSubscribe(this);
            }
        }

        @Override
        public void onSuccess(T value) {
            actual.onSuccess(value);
        }

        @Override
        public void onError(Throwable e) {
            actual.onError(e);
        }
    }

}
