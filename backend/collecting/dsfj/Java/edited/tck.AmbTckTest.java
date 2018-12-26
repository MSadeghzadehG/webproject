

package io.reactivex.tck;

import java.util.Arrays;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class AmbTckTest extends BaseTck<Long> {

    @SuppressWarnings("unchecked")
    @Override
    public Publisher<Long> createPublisher(long elements) {
        return
            Flowable.amb(Arrays.asList(
                    Flowable.fromIterable(iterate(elements)),
                    Flowable.<Long>never()
                )
            )
        ;
    }
}
