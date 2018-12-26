

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.*;

@Test
public class ConcatWithMaybeTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.range(1, Math.max(0, (int)elements - 1))
                .concatWith(Maybe.just((int)elements))
            ;
    }
}
