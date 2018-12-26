

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.schedulers.Timed;

@Test
public class TimeIntervalTckTest extends BaseTck<Timed<Integer>> {

    @Override
    public Publisher<Timed<Integer>> createPublisher(long elements) {
        return
                Flowable.range(0, (int)elements).timeInterval()
        ;
    }
}
