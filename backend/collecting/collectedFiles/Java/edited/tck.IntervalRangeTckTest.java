

package io.reactivex.tck;

import java.util.concurrent.TimeUnit;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class IntervalRangeTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(long elements) {
        return
            Flowable.intervalRange(0, elements, 0, 1, TimeUnit.MILLISECONDS)
            .onBackpressureBuffer()
        ;
    }
}
