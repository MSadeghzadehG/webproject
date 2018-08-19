

package io.reactivex.tck;

import java.util.Map;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.internal.functions.Functions;

@Test
public class ToMapTckTest extends BaseTck<Map<Integer, Integer>> {

    @Override
    public Publisher<Map<Integer, Integer>> createPublisher(final long elements) {
        return
                Flowable.range(1, 1000).toMap(Functions.<Integer>identity()).toFlowable()
            ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1;
    }
}
