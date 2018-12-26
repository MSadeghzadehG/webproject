

package io.reactivex.internal.operators.flowable;

import org.reactivestreams.Subscriber;

import io.reactivex.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.plugins.RxJavaPlugins;


public final class FlowableLift<R, T> extends AbstractFlowableWithUpstream<T, R> {
    
    final FlowableOperator<? extends R, ? super T> operator;

    public FlowableLift(Flowable<T> source, FlowableOperator<? extends R, ? super T> operator) {
        super(source);
        this.operator = operator;
    }

    @Override
    public void subscribeActual(Subscriber<? super R> s) {
        try {
            Subscriber<? super T> st = operator.apply(s);

            if (st == null) {
                throw new NullPointerException("Operator " + operator + " returned a null Subscriber");
            }

            source.subscribe(st);
        } catch (NullPointerException e) {             throw e;
        } catch (Throwable e) {
            Exceptions.throwIfFatal(e);
                                    RxJavaPlugins.onError(e);

            NullPointerException npe = new NullPointerException("Actually not, but can't throw other exceptions due to RS");
            npe.initCause(e);
            throw npe;
        }
    }
}
