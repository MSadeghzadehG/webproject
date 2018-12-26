

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class JustTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(long elements) {
        return
                Flowable.just(0L)
        ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1L;
    }
}
