

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.*;

@Test
public class CreateTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(final long elements) {
        return
            Flowable.create(new FlowableOnSubscribe<Long>() {
                @Override
                public void subscribe(FlowableEmitter<Long> e) throws Exception {
                    for (long i = 0; i < elements && !e.isCancelled(); i++) {
                        e.onNext(i);
                    }
                    e.onComplete();
                }
            }, BackpressureStrategy.BUFFER)
        ;
    }
}
