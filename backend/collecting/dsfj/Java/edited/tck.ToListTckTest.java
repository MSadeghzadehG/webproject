

package io.reactivex.tck;

import java.util.List;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;

@Test
public class ToListTckTest extends BaseTck<List<Integer>> {

    @Override
    public Publisher<List<Integer>> createPublisher(final long elements) {
        return
                Flowable.range(1, 1000).toList().toFlowable()
            ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1;
    }
}
