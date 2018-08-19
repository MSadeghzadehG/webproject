

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class IsEmptyTckTest extends BaseTck<Boolean> {

    @Override
    public Publisher<Boolean> createPublisher(final long elements) {
        return
                Flowable.range(1, 10).isEmpty().toFlowable()
            ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1;
    }
}
