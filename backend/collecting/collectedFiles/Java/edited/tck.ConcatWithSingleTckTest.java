

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.*;

@Test
public class ConcatWithSingleTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.range(1, Math.max(0, (int)elements - 1))
                .concatWith(Single.just((int)elements))
            ;
    }
}
