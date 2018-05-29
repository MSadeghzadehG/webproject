

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class SingleTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(final long elements) {
        return
                Flowable.just(1).singleElement().toFlowable()
            ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1;
    }
}
