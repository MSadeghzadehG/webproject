

package io.reactivex.tck;

import java.util.concurrent.Callable;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class FromCallableTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(final long elements) {
        return
                Flowable.fromCallable(new Callable<Long>() {
                    @Override
                    public Long call() throws Exception {
                        return 1L;
                    }
                }
                )
            ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1;
    }
}
