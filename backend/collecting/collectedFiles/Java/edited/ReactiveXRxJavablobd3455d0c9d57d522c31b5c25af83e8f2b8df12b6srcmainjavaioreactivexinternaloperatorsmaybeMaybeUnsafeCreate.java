

package io.reactivex.internal.operators.maybe;

import io.reactivex.*;


public final class MaybeUnsafeCreate<T> extends AbstractMaybeWithUpstream<T, T> {

    public MaybeUnsafeCreate(MaybeSource<T> source) {
        super(source);
    }

    @Override
    protected void subscribeActual(MaybeObserver<? super T> observer) {
        source.subscribe(observer);
    }

}
