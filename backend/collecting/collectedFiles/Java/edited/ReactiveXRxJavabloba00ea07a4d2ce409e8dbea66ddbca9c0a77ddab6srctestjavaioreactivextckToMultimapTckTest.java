

package io.reactivex.tck;

import java.util.*;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.internal.functions.Functions;

@Test
public class ToMultimapTckTest extends BaseTck<Map<Integer, Collection<Integer>>> {

    @Override
    public Publisher<Map<Integer, Collection<Integer>>> createPublisher(final long elements) {
        return
                Flowable.range(1, 1000).toMultimap(Functions.<Integer>identity()).toFlowable()
            ;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1;
    }
}
