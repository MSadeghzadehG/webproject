

package io.reactivex.tck;

import java.util.List;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.functions.BiConsumer;
import io.reactivex.internal.functions.Functions;

@Test
public class CollectTckTest extends BaseTck<List<Integer>> {

    @Override
    public Publisher<List<Integer>> createPublisher(final long elements) {
        return
                Flowable.range(1, 1000).collect(Functions.<Integer>createArrayList(128), new BiConsumer<List<Integer>, Integer>() {
                    @Override
                    public void accept(List<Integer> a, Integer b) throws Exception {
                        a.add(b);
                    }
                }).toFlowable()
            ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1;
    }
}
