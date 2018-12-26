

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.*;

@Test
public class MergeWithMaybeTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(long elements) {
        if (elements == 0) {
            return Flowable.<Long>empty()
                    .mergeWith(Maybe.<Long>empty());
        }
        return
            Flowable.rangeLong(1, elements - 1)
            .mergeWith(Maybe.just(elements))
        ;
    }
}
