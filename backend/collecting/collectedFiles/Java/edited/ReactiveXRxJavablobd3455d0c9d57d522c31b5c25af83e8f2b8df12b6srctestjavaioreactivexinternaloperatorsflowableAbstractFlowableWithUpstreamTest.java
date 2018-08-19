

package io.reactivex.internal.operators.flowable;

import static org.junit.Assert.assertSame;

import org.junit.Test;

import io.reactivex.Flowable;
import io.reactivex.internal.functions.Functions;
import io.reactivex.internal.fuseable.HasUpstreamPublisher;

public class AbstractFlowableWithUpstreamTest {

    @SuppressWarnings("unchecked")
    @Test
    public void source() {
        Flowable<Integer> o = Flowable.just(1);

        assertSame(o, ((HasUpstreamPublisher<Integer>)o.map(Functions.<Integer>identity())).source());
    }
}
