

package io.reactivex.tck;

import java.util.concurrent.*;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class TimerTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(final long elements) {
        return
                Flowable.timer(1, TimeUnit.MILLISECONDS)
                .onBackpressureLatest()
            ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1;
    }
}
