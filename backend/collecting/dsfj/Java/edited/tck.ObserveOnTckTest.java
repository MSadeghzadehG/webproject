

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.schedulers.Schedulers;

@Test
public class ObserveOnTckTest extends BaseTck<Integer> {

    public ObserveOnTckTest() {
        super(100L);
    }

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.range(0, (int)elements).observeOn(Schedulers.single())
        ;
    }
}
