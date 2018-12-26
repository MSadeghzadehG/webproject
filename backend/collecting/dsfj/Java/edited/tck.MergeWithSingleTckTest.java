

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.*;

@Test
public class MergeWithSingleTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(long elements) {
        if (elements == 0) {
            return Flowable.empty();
        }
        return
            Flowable.rangeLong(1, elements - 1)
            .mergeWith(Single.just(elements))
        ;
    }
}
