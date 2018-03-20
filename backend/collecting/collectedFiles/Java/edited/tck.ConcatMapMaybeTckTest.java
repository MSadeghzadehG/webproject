

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.*;
import io.reactivex.functions.Function;

@Test
public class ConcatMapMaybeTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.range(0, (int)elements)
                .concatMapMaybe(new Function<Integer, Maybe<Integer>>() {
                    @Override
                    public Maybe<Integer> apply(Integer v) throws Exception {
                        return Maybe.just(v);
                    }
                })
            ;
    }
}
