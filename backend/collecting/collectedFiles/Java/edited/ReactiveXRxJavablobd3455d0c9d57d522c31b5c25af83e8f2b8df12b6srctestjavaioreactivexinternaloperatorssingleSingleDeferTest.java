

package io.reactivex.internal.operators.single;

import java.util.concurrent.Callable;

import org.junit.Test;

import io.reactivex.Single;

public class SingleDeferTest {

    @Test
    public void normal() {

        Single<Integer> s = Single.defer(new Callable<Single<Integer>>() {
            int counter;
            @Override
            public Single<Integer> call() throws Exception {
                return Single.just(++counter);
            }
        });

        for (int i = 1; i < 33; i++) {
            s.test().assertResult(i);
        }
    }
}
