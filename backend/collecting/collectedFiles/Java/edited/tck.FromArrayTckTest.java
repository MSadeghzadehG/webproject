

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class FromArrayTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(long elements) {
        return
                Flowable.fromArray(array(elements))
        ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1024 * 1024;
    }
}
