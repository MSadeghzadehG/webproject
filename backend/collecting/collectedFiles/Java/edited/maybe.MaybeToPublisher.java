

package io.reactivex.internal.operators.maybe;

import org.reactivestreams.Publisher;

import io.reactivex.MaybeSource;
import io.reactivex.functions.Function;


public enum MaybeToPublisher implements Function<MaybeSource<Object>, Publisher<Object>> {
    INSTANCE;

    @SuppressWarnings({ "rawtypes", "unchecked" })
    public static <T> Function<MaybeSource<T>, Publisher<T>> instance() {
        return (Function)INSTANCE;
    }

    @Override
    public Publisher<Object> apply(MaybeSource<Object> t) throws Exception {
        return new MaybeToFlowable<Object>(t);
    }
}
