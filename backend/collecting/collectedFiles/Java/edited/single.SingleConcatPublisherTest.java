
package io.reactivex.internal.operators.single;

import java.util.concurrent.Callable;

import org.junit.Test;

import io.reactivex.*;

public class SingleConcatPublisherTest {

    @Test
    public void scalar() {
        Single.concat(Flowable.just(Single.just(1)))
        .test()
        .assertResult(1);
    }

    @Test
    public void callable() {
        Single.concat(Flowable.fromCallable(new Callable<Single<Integer>>() {
            @Override
            public Single<Integer> call() throws Exception {
                return Single.just(1);
            }
        }))
        .test()
        .assertResult(1);
    }
}
