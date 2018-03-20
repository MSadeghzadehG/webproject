

package io.reactivex.internal.operators.completable;

import io.reactivex.*;

public final class CompletableFromUnsafeSource extends Completable {

    final CompletableSource source;

    public CompletableFromUnsafeSource(CompletableSource source) {
        this.source = source;
    }

    @Override
    protected void subscribeActual(CompletableObserver observer) {
        source.subscribe(observer);
    }
}
