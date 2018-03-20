

package io.reactivex.tck;

import java.util.List;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class BufferBoundaryTckTest extends BaseTck<List<Long>> {

    @Override
    public Publisher<List<Long>> createPublisher(long elements) {
        return
            Flowable.fromIterable(iterate(elements))
            .buffer(Flowable.just(1).concatWith(Flowable.<Integer>never()))
            .onBackpressureLatest()
        ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1;
    }
}
