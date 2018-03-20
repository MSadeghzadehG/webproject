

package io.reactivex.internal.operators.maybe;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.internal.fuseable.*;
import io.reactivex.plugins.RxJavaPlugins;


public final class MaybeIsEmptySingle<T> extends Single<Boolean>
implements HasUpstreamMaybeSource<T>, FuseToMaybe<Boolean> {

    final MaybeSource<T> source;

    public MaybeIsEmptySingle(MaybeSource<T> source) {
        this.source = source;
    }

    @Override
    public MaybeSource<T> source() {
        return source;
    }

    @Override
    public Maybe<Boolean> fuseToMaybe() {
        return RxJavaPlugins.onAssembly(new MaybeIsEmpty<T>(source));
    }

    @Override
    protected void subscribeActual(SingleObserver<? super Boolean> observer) {
        source.subscribe(new IsEmptyMaybeObserver<T>(observer));
    }

    static final class IsEmptyMaybeObserver<T>
    implements MaybeObserver<T>, Disposable {

        final SingleObserver<? super Boolean> actual;

        Disposable d;

        IsEmptyMaybeObserver(SingleObserver<? super Boolean> actual) {
            this.actual = actual;
        }

        @Override
        public void dispose() {
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
                this.d = d;

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onSuccess(T value) {
            d = DisposableHelper.DISPOSED;
            actual.onSuccess(false);
        }

        @Override
        public void onError(Throwable e) {
            d = DisposableHelper.DISPOSED;
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            d = DisposableHelper.DISPOSED;
            actual.onSuccess(true);
        }
    }
}
