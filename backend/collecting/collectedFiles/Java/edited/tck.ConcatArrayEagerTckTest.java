

package io.reactivex.tck;

import java.util.Arrays;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class ConcatArrayEagerTckTest extends BaseTck<Long> {

    @SuppressWarnings("unchecked")
    @Override
    public Publisher<Long> createPublisher(long elements) {
        return
            Flowable.concatEager(Arrays.asList(
                    Flowable.fromIterable(iterate(elements / 2)),
                    Flowable.fromIterable(iterate(elements - elements / 2))
                )
            )
        ;
    }
}
