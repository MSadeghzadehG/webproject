

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.functions.Predicate;

@Test
public class AllTckTest extends BaseTck<Boolean> {

    @Override
    public Publisher<Boolean> createPublisher(final long elements) {
        return
                Flowable.range(1, 1000).all(new Predicate<Integer>() {
                    @Override
                    public boolean test(Integer e) throws Exception {
                        return e < 800;
                    }
                }).toFlowable()
            ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1;
    }
}
