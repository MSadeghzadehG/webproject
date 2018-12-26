

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.functions.Function;

@Test
public class ConcatMapTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.range(0, (int)elements)
                .concatMap(new Function<Integer, Publisher<Integer>>() {
                    @Override
                    public Publisher<Integer> apply(Integer v) throws Exception {
                        return Flowable.just(v);
                    }
                })
            ;
    }
}
