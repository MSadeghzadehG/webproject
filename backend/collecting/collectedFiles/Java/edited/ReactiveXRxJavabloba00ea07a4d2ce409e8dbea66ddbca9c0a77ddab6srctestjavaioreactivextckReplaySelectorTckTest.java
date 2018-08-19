

package io.reactivex.tck;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.functions.Function;
import io.reactivex.internal.functions.Functions;

@Test
public class ReplaySelectorTckTest extends BaseTck<Integer> {

    @SuppressWarnings({ "unchecked", "rawtypes" })
    @Override
    public Publisher<Integer> createPublisher(long elements) {
        return
                Flowable.range(0, (int)elements).replay((Function)Functions.identity())
        ;
    }
}
