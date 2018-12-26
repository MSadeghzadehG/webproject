

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.schedulers.Schedulers;

@Test
public class SubscribeOnTckTest extends BaseTck<Integer> {

    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.range(0, (int)elements).subscribeOn(Schedulers.single())
        ;
    }
}
