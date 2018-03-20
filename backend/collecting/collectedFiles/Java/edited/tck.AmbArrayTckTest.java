

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class AmbArrayTckTest extends BaseTck<Long> {

    @SuppressWarnings("unchecked")
    @Override
    public Publisher<Long> createPublisher(long elements) {
        return
                Flowable.ambArray(
                        Flowable.fromIterable(iterate(elements)),
                        Flowable.<Long>never()
                )
            ;
    }
}
