

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.*;
import io.reactivex.annotations.Experimental;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Action;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.plugins.RxJavaPlugins;


@Experimental
public final class MaybeDoFinally<T> extends AbstractMaybeWithUpstream<T, T> {

    final Action onFinally;

    public MaybeDoFinally(MaybeSource<T> source, Action onFinally) {
        super(source);
        this.onFinally = onFinally;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> s) {
        source.subscribe(new DoFinallyObserver<T>(s, onFinally));
    }

    static final class DoFinallyObserver<T> extends AtomicInteger implements MaybeObserver<T>, Disposable {

        private static final long serialVersionUID = 4109457741734051389L;

        final MaybeObserver<? super T> actual;

        final Action onFinally;

        Disposable d;

        DoFinallyObserver(MaybeObserver<? super T> actual, Action onFinally) {
            this.actual = actual;
            this.onFinally = onFinally;
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
            runFinally();
        }

        @Override
        public void onError(Throwable t) {
            actual.onError(t);
            runFinally();
        }

        @Override
        public void onComplete() {
            actual.onComplete();
            runFinally();
        }

        @Override
        public void dispose() {
            d.dispose();
            runFinally();
        }

        @Override
        public boolean isDisposed() {
            return d.isDisposed();
        }

        void runFinally() {
            if (compareAndSet(0, 1)) {
                try {
                    onFinally.run();
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    RxJavaPlugins.onError(ex);
                }
            }
        }
    }
}
