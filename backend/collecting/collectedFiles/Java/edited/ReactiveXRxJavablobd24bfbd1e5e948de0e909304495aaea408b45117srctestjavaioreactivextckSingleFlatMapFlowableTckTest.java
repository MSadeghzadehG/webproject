

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.*;
import io.reactivex.functions.Function;

@Test
public class SingleFlatMapFlowableTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(final long elements) {
        return
                Single.just(1).hide().flatMapPublisher(new Function<Integer, Publisher<Integer>>() {
                    @Override
                    public Publisher<Integer> apply(Integer v)
                            throws Exception {
                        return Flowable.range(0, (int)elements);
                    }
                })
        ;
    }
}
