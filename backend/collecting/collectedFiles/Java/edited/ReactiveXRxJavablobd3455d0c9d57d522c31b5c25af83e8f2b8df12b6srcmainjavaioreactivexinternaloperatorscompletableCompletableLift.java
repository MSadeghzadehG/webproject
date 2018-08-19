

package io.reactivex.internal.operators.completable;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.plugins.RxJavaPlugins;

public final class CompletableLift extends Completable {

    final CompletableSource source;

    final CompletableOperator onLift;

    public CompletableLift(CompletableSource source, CompletableOperator onLift) {
        this.source = source;
        this.onLift = onLift;
    }

    @Override
    protected void subscribeActual(CompletableObserver s) {
        try {
            
            CompletableObserver sw = onLift.apply(s);

            source.subscribe(sw);
        } catch (NullPointerException ex) {             throw ex;
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            RxJavaPlugins.onError(ex);
        }
    }

}
