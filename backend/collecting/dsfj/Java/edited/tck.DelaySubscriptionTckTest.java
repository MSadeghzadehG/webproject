

package io.reactivex.tck;

import java.util.concurrent.TimeUnit;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class DelaySubscriptionTckTest extends BaseTck<Integer> {

    public DelaySubscriptionTckTest() {
        super(200L);
    }

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.range(0, (int)elements).delaySubscription(1, TimeUnit.MILLISECONDS)
        ;
    }
}
