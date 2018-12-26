

package io.reactivex.internal.operators.single;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.functions.Function;

public class SingleFlatMapCompletableTest {

    @Test
    public void dispose() {
        TestHelper.checkDisposed(Single.just(1).flatMapCompletable(new Function<Integer, Completable>() {
            @Override
            public Completable apply(Integer v) throws Exception {
                return Completable.complete();
            }
        }));
    }
}
