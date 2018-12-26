

package io.reactivex.internal.operators.maybe;

import static org.junit.Assert.*;
import org.junit.Test;

import io.reactivex.Maybe;
import io.reactivex.internal.fuseable.HasUpstreamMaybeSource;

public class MaybeIsEmptySingleTest {

    @Test
    public void source() {
        Maybe<Integer> m = Maybe.just(1);

        assertSame(m, (((HasUpstreamMaybeSource<?>)m.isEmpty()).source()));
    }
}
