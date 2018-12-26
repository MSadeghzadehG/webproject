

package io.reactivex.tck;

import java.util.Collections;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.functions.Function;

@Test
public class ConcatMapIterableTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.range(0, (int)elements)
                .concatMapIterable(new Function<Integer, Iterable<Integer>>() {
                    @Override
                    public Iterable<Integer> apply(Integer v) throws Exception {
                        return Collections.singletonList(v);
                    }
                })
            ;
    }
}
