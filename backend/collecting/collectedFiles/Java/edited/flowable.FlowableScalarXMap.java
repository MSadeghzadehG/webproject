

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.Callable;

import org.reactivestreams.*;

import io.reactivex.Flowable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Function;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.internal.subscriptions.*;
import io.reactivex.plugins.RxJavaPlugins;


public final class FlowableScalarXMap {

    
    private FlowableScalarXMap() {
        throw new IllegalStateException("No instances!");
    }

    
    @SuppressWarnings("unchecked")
    public static <T, R> boolean tryScalarXMapSubscribe(Publisher<T> source,
            Subscriber<? super R> subscriber,
            Function<? super T, ? extends Publisher<? extends R>> mapper) {
        if (source instanceof Callable) {
            T t;

            try {
                t = ((Callable<T>)source).call();
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                EmptySubscription.error(ex, subscriber);
                return true;
            }

            if (t == null) {
                EmptySubscription.complete(subscriber);
                return true;
            }

            Publisher<? extends R> r;

            try {
                r = ObjectHelper.requireNonNull(mapper.apply(t), "The mapper returned a null Publisher");
            } catch (Throwable ex) {
                Exceptions.throwIfFatal(ex);
                EmptySubscription.error(ex, subscriber);
                return true;
            }

            if (r instanceof Callable) {
                R u;

                try {
                    u = ((Callable<R>)r).call();
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    EmptySubscription.error(ex, subscriber);
                    return true;
                }

                if (u == null) {
                    EmptySubscription.complete(subscriber);
                    return true;
                }
                subscriber.onSubscribe(new ScalarSubscription<R>(subscriber, u));
            } else {
                r.subscribe(subscriber);
            }

            return true;
        }
        return false;
    }

    
    public static <T, U> Flowable<U> scalarXMap(final T value, final Function<? super T, ? extends Publisher<? extends U>> mapper) {
        return RxJavaPlugins.onAssembly(new ScalarXMapFlowable<T, U>(value, mapper));
    }

    
    static final class ScalarXMapFlowable<T, R> extends Flowable<R> {

        final T value;

        final Function<? super T, ? extends Publisher<? extends R>> mapper;

        ScalarXMapFlowable(T value,
                Function<? super T, ? extends Publisher<? extends R>> mapper) {
            this.value = value;
            this.mapper = mapper;
        }

        @SuppressWarnings("unchecked")
        @Override
        public void subscribeActual(Subscriber<? super R> s) {
            Publisher<? extends R> other;
            try {
                other = ObjectHelper.requireNonNull(mapper.apply(value), "The mapper returned a null Publisher");
            } catch (Throwable e) {
                EmptySubscription.error(e, s);
                return;
            }
            if (other instanceof Callable) {
                R u;

                try {
                    u = ((Callable<R>)other).call();
                } catch (Throwable ex) {
                    Exceptions.throwIfFatal(ex);
                    EmptySubscription.error(ex, s);
                    return;
                }

                if (u == null) {
                    EmptySubscription.complete(s);
                    return;
                }
                s.onSubscribe(new ScalarSubscription<R>(s, u));
            } else {
                other.subscribe(s);
            }
        }
    }
}
