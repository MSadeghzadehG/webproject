

package io.reactivex.tck;

import java.util.concurrent.TimeUnit;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class DelayTckTest extends BaseTck<Integer> {

    public DelayTckTest() {
        super(100L);
    }

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
            Flowable.range(0, (int)elements).delay(1, TimeUnit.MILLISECONDS)
        ;
    }
}
