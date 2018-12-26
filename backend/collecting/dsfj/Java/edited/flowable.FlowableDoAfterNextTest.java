
package io.reactivex.flowable;

import static org.junit.Assert.assertEquals;

import java.util.concurrent.atomic.AtomicInteger;

import org.junit.Test;

import io.reactivex.functions.Consumer;

public class FlowableDoAfterNextTest {

    @Test
    public void testIfFunctionThrowsThatNoMoreEventsAreProcessed() {
        final AtomicInteger count = new AtomicInteger();
        final RuntimeException e = new RuntimeException();
        Burst.items(1, 2).create()
            .doAfterNext(new Consumer<Integer>() {
                @Override
                public void accept(Integer t) throws Exception {
                    count.incrementAndGet();
                    throw e;
                }})
            .test()
            .assertError(e)
            .assertValue(1);
        assertEquals(1, count.get());
    }
}
