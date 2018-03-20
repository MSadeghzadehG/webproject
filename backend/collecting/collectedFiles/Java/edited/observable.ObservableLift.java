

package io.reactivex.internal.operators.observable;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.plugins.RxJavaPlugins;


public final class ObservableLift<R, T> extends AbstractObservableWithUpstream<T, R> {
    
    final ObservableOperator<? extends R, ? super T> operator;

    public ObservableLift(ObservableSource<T> source, ObservableOperator<? extends R, ? super T> operator) {
        super(source);
        this.operator = operator;
    }

    @Override
    public void subscribeActual(Observer<? super R> s) {
        Observer<? super T> observer;
        try {
            observer = ObjectHelper.requireNonNull(operator.apply(s), "Operator " + operator + " returned a null Observer");
        } catch (NullPointerException e) {             throw e;
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
                                    RxJavaPlugins.onError(e);

            NullPointerException npe = new NullPointerException("Actually not, but can't throw other exceptions due to RS");
            npe.initCause(e);
            throw npe;
        }

        source.subscribe(observer);
    }
}
