

package io.reactivex.internal.operators.maybe;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.fuseable.FuseToMaybe;
import io.reactivex.plugins.RxJavaPlugins;


public final class MaybeIgnoreElementCompletable<T> extends Completable implements FuseToMaybe<T> {

    final MaybeSource<T> source;

    public MaybeIgnoreElementCompletable(MaybeSource<T> source) {
        this.source = source;
    }

    @Override
    protected void subscribeActual(CompletableObserver observer) {
        source.subscribe(new IgnoreMaybeObserver<T>(observer));
    }

    @Override
    public Maybe<T> fuseToMaybe() {
        return RxJavaPlugins.onAssembly(new MaybeIgnoreElement<T>(source));
    }

    static final class IgnoreMaybeObserver<T> implements MaybeObserver<T>, Disposable {

        final CompletableObserver actual;

        Disposable d;

        IgnoreMaybeObserver(CompletableObserver actual) {
            this.actual = actual;
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
            d = DisposableHelper.DISPOSED;
            actual.onComplete();
        }

        @Override
        public void onError(Throwable e) {
            d = DisposableHelper.DISPOSED;
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            d = DisposableHelper.DISPOSED;
            actual.onComplete();
        }

        @Override
        public boolean isDisposed() {
            return d.isDisposed();
        }

        @Override
        public void dispose() {
            d.dispose();
            d = DisposableHelper.DISPOSED;
        }

    }

}
