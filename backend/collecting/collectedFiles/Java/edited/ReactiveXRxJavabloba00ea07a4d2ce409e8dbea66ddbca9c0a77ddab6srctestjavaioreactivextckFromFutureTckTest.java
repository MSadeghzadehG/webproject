

package io.reactivex.tck;

import java.util.concurrent.*;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class FromFutureTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(final long elements) {
        FutureTask<Long> ft = new FutureTask<Long>(new Callable<Long>() {
            @Override
            public Long call() throws Exception {
                return 1L;
            }
        });

        ft.run();
        return Flowable.fromFuture(ft);
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1;
    }
}
