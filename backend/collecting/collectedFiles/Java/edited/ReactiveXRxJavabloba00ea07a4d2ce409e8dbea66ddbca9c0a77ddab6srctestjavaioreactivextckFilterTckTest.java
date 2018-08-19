

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.functions.Predicate;

@Test
public class FilterTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.range(0, (int)elements * 2).filter(new Predicate<Integer>() {
                    @Override
                    public boolean test(Integer v) throws Exception {
                        return (v & 1) == 0;
                    }
                })
        ;
    }
}
