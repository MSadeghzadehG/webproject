

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.*;

@Test
public class CompletableAndThenPublisherTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(final long elements) {
        return
                Completable.complete().hide().andThen(Flowable.range(0, (int)elements))
        ;
    }
}
