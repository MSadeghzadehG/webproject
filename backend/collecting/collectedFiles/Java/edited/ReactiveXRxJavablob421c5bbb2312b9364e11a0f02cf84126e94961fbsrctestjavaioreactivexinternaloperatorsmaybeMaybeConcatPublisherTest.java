
package io.reactivex.internal.operators.maybe;

import java.util.concurrent.Callable;

import org.junit.Test;

import io.reactivex.*;

public class MaybeConcatPublisherTest {

    @Test
    public void scalar() {
        Maybe.concat(Flowable.just(Maybe.just(1)))
        .test()
        .assertResult(1);
    }

    @Test
    public void callable() {
        Maybe.concat(Flowable.fromCallable(new Callable<Maybe<Integer>>() {
            @Override
            public Maybe<Integer> call() throws Exception {
                return Maybe.just(1);
            }
        }))
        .test()
        .assertResult(1);
    }
}
