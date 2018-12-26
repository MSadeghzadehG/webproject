

package io.reactivex.internal.operators.completable;

import io.reactivex.*;
import io.reactivex.disposables.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Action;

public final class CompletableFromAction extends Completable {

    final Action run;

    public CompletableFromAction(Action run) {
        this.run = run;
    }

    @Override
    protected void subscribeActual(CompletableObserver s) {
        Disposable d = Disposables.empty();
        s.onSubscribe(d);
        try {
            run.run();
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
            if (!d.isDisposed()) {
                s.onError(e);
            }
            return;
        }
        if (!d.isDisposed()) {
            s.onComplete();
        }
    }

}
