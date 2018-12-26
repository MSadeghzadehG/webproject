

package io.reactivex.internal.operators.completable;

import io.reactivex.*;
import io.reactivex.internal.disposables.EmptyDisposable;

public final class CompletableNever extends Completable {
    public static final Completable INSTANCE = new CompletableNever();

    private CompletableNever() {
    }

    @Override
    protected void subscribeActual(CompletableObserver s) {
        s.onSubscribe(EmptyDisposable.NEVER);
    }

}
