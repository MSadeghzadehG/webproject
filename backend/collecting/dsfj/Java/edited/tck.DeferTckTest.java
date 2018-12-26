

package io.reactivex.tck;

import java.util.concurrent.Callable;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class DeferTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(final long elements) {
        return
                Flowable.defer(new Callable<Publisher<Long>>() {
                    @Override
                    public Publisher<Long> call() throws Exception {
                        return Flowable.fromIterable(iterate(elements));
                    }
                }
                )
            ;
    }
}
