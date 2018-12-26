

package io.reactivex.tck;

import java.util.Arrays;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.functions.Function;

@Test
public class CombineLatestIterableTckTest extends BaseTck<Long> {

    @SuppressWarnings("unchecked")
    @Override
    public Publisher<Long> createPublisher(long elements) {
        return
            Flowable.combineLatest(Arrays.asList(
                    Flowable.just(1L),
                    Flowable.fromIterable(iterate(elements))
                ),
                new Function<Object[], Long>() {
                    @Override
                    public Long apply(Object[] a) throws Exception {
                        return (Long)a[0];
                    }
                }
            )
        ;
    }
}
