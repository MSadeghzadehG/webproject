

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.*;

@Test
public class MergeWithMaybeEmptyTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(long elements) {
        return
            Flowable.rangeLong(1, elements)
            .mergeWith(Maybe.<Long>empty())
        ;
    }
}
