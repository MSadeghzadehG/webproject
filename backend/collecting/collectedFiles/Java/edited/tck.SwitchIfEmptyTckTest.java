

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class SwitchIfEmptyTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.<Integer>empty().switchIfEmpty(Flowable.range(1, (int)elements))
            ;
    }
}