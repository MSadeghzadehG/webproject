

package io.reactivex.tck;

import java.util.List;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class BufferExactSizeTckTest extends BaseTck<List<Long>> {

    @Override
    public Publisher<List<Long>> createPublisher(long elements) {
        return
            Flowable.fromIterable(iterate(elements * 2))
            .buffer(2)
        ;
    }
}
