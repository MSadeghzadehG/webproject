

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class EmptyTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(final long elements) {
        return Flowable.<Long>empty();
    }

    @Override
    public long maxElementsFromPublisher() {
        return 0;
    }
}
