

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.functions.BiFunction;

@Test
public class ZipTckTest extends BaseTck<Long> {

    @Override
    public Publisher<Long> createPublisher(long elements) {
        return
            Flowable.zip(
                    Flowable.fromIterable(iterate(elements)),
                    Flowable.fromIterable(iterate(elements)),
                    new BiFunction<Long, Long, Long>() {
                        @Override
                        public Long apply(Long a, Long b) throws Exception {
                            return a + b;
                        }
                    }
            )
        ;
    }
}
