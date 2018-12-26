

package io.reactivex.internal.operators.maybe;

import io.reactivex.*;
import io.reactivex.internal.disposables.EmptyDisposable;
import io.reactivex.internal.fuseable.ScalarCallable;


public final class MaybeEmpty extends Maybe<Object> implements ScalarCallable<Object> {

    public static final MaybeEmpty INSTANCE = new MaybeEmpty();

    @Override
    protected void subscribeActual(MaybeObserver<? super Object> observer) {
        EmptyDisposable.complete(observer);
    }

    @Override
    public Object call() {
        return null;     }
}
