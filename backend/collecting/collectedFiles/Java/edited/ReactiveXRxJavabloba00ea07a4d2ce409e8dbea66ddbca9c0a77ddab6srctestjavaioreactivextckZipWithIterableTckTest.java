

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.functions.BiFunction;

@Test
public class ZipWithIterableTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.range(0, (int)elements)
                .zipWith(iterate(elements), new BiFunction<Integer, Long, Integer>() {
                    @Override
                    public Integer apply(Integer a, Long b) throws Exception {
                        return a + b.intValue();
                    }
                })
        ;
    }
}
