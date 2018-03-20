

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class IgnoreElementsTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(final long elements) {
        return
                Flowable.range(1, 1000).ignoreElements().<Integer>toFlowable()
            ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 0;
    }
}
