

package io.reactivex.internal.operators.maybe;

import org.junit.Test;
import static org.junit.Assert.*;

import io.reactivex.Maybe;
import io.reactivex.internal.functions.Functions;
import io.reactivex.internal.fuseable.HasUpstreamMaybeSource;

public class AbstractMaybeWithUpstreamTest {

    @SuppressWarnings("unchecked")
    @Test
    public void upstream() {
        Maybe<Integer> source = Maybe.just(1);

        assertSame(source, ((HasUpstreamMaybeSource<Integer>)source.map(Functions.<Integer>identity())).source());
    }
}
