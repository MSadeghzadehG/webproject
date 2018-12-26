

package io.reactivex.internal.operators.maybe;

import io.reactivex.*;
import io.reactivex.internal.disposables.EmptyDisposable;


public final class MaybeNever extends Maybe<Object> {

    public static final MaybeNever INSTANCE = new MaybeNever();

    @Override
    protected void subscribeActual(MaybeObserver<? super Object> observer) {
        observer.onSubscribe(EmptyDisposable.NEVER);
    }
}
