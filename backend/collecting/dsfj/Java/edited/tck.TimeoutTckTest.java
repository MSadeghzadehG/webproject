

package io.reactivex.tck;

import java.util.concurrent.TimeUnit;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class TimeoutTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.range(0, (int)elements).timeout(1, TimeUnit.DAYS)
        ;
    }
}
