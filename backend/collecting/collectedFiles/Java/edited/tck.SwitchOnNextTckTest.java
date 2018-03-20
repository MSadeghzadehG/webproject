

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class SwitchOnNextTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(long elements) {
        return
            Flowable.switchOnNext(Flowable.just(
                    Flowable.fromIterable(iterate(elements)))
            )
        ;
    }
}
