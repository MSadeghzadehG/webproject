

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class SequenceEqualTckTest extends BaseTck<Boolean> {

    @Override
    public Publisher<Boolean> createPublisher(final long elements) {
        return
                Flowable.sequenceEqual(
                        Flowable.range(1, 1000),
                        Flowable.range(1, 1001))
                .toFlowable()
            ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1;
    }
}
