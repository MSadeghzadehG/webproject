

package io.reactivex.tck;

import java.util.List;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.functions.Function;
import io.reactivex.internal.functions.Functions;

@Test
public class WindowBoundaryTckTest extends BaseTck<List<Long>> {

    @SuppressWarnings({ "rawtypes", "unchecked" })
    @Override
    public Publisher<List<Long>> createPublisher(long elements) {
        return
            Flowable.fromIterable(iterate(elements))
            .window(Flowable.just(1).concatWith(Flowable.<Integer>never()))
            .onBackpressureBuffer()
            .flatMap((Function)Functions.identity())
        ;
    }
}
