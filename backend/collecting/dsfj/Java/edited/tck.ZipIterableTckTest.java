

package io.reactivex.tck;

import java.util.Arrays;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.functions.Function;

@Test
public class ZipIterableTckTest extends BaseTck<Long> {

    @SuppressWarnings("unchecked")
    @Override
    public Publisher<Long> createPublisher(long elements) {
        return
            Flowable.zip(Arrays.asList(
                    Flowable.fromIterable(iterate(elements)),
                    Flowable.fromIterable(iterate(elements))
                ),
                new Function<Object[], Long>() {
                    @Override
                    public Long apply(Object[] a) throws Exception {
                        return (Long)a[0] + (Long)a[1];
                    }
                }
            )
        ;
    }
}
