

package io.reactivex.internal.operators.maybe;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.*;
import io.reactivex.functions.*;
import io.reactivex.internal.disposables.*;
import io.reactivex.plugins.RxJavaPlugins;


public final class MaybePeek<T> extends AbstractMaybeWithUpstream<T, T> {

    final Consumer<? super Disposable> onSubscribeCall;

    final Consumer<? super T> onSuccessCall;

    final Consumer<? super Throwable> onErrorCall;

    final Action onCompleteCall;

    final Action onAfterTerminate;

    final Action onDisposeCall;

    public MaybePeek(MaybeSource<T> source, Consumer<? super Disposable> onSubscribeCall,
            Consumer<? super T> onSuccessCall, Consumer<? super Throwable> onErrorCall, Action onCompleteCall,
            Action onAfterTerminate, Action onDispose) {
        super(source);
        this.onSubscribeCall = onSubscribeCall;
        this.onSuccessCall = onSuccessCall;
        this.onErrorCall = onErrorCall;
        this.onCompleteCall = onCompleteCall;
        this.onAfterTerminate = onAfterTerminate;
        this.onDisposeCall = onDispose;
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        source.subscribe(new MaybePeekObserver<T>(observer, this));
    }

    static final class MaybePeekObserver<T> implements MaybeObserver<T>, Disposable {
        final MaybeObserver<? super T> actual;

        final MaybePeek<T> parent;

        Disposable d;

        MaybePeekObserver(MaybeObserver<? super T> actual, MaybePeek<T> parent) {
            this.actual = actual;
            this.parent = parent;
        }

        @Override
        public void dispose() {
            try {
                parent.onDisposeCall.run();
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                RxJavaPlugins.onError(ex);
            }

            d.dispose();
            d = DisposableHelper.DISPOSED;
        }

        @Override
        public boolean isDisposed() {
            return d.isDisposed();
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                try {
                    parent.onSubscribeCall.accept(d);
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    d.dispose();
                    this.d = DisposableHelper.DISPOSED;
                    EmptyDisposable.error(ex, actual);
                    return;
                }

                this.d = d;

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onSuccess(T value) {
            if (this.d == DisposableHelper.DISPOSED) {
                return;
            }
            try {
                parent.onSuccessCall.accept(value);
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                onErrorInner(ex);
                return;
            }
            this.d = DisposableHelper.DISPOSED;

            actual.onSuccess(value);

            onAfterTerminate();
        }

        @Override
        public void onError(Throwable e) {
            if (this.d == DisposableHelper.DISPOSED) {
                RxJavaPlugins.onError(e);
                return;
            }

            onErrorInner(e);
        }

        void onErrorInner(Throwable e) {
            try {
                parent.onErrorCall.accept(e);
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                e = new CompositeException(e, ex);
            }

            this.d = DisposableHelper.DISPOSED;

            actual.onError(e);

            onAfterTerminate();
        }

        @Override
        public void onComplete() {
            if (this.d == DisposableHelper.DISPOSED) {
                return;
            }

            try {
                parent.onCompleteCall.run();
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                onErrorInner(ex);
                return;
            }
            this.d = DisposableHelper.DISPOSED;

            actual.onComplete();

            onAfterTerminate();
        }

        void onAfterTerminate() {
            try {
                parent.onAfterTerminate.run();
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                RxJavaPlugins.onError(ex);
            }
        }
    }
}
