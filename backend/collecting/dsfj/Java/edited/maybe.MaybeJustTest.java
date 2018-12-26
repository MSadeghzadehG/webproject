

package io.reactivex.internal.operators.maybe;

import static org.junit.Assert.*;

import org.junit.Test;

import io.reactivex.Maybe;
import io.reactivex.internal.fuseable.ScalarCallable;

public class MaybeJustTest {

    @SuppressWarnings("unchecked")
    @Test
    public void scalarCallable() {
        Maybe<Integer> m = Maybe.just(1);

        assertTrue(m.getClass().toString(), m instanceof ScalarCallable);

        assertEquals(1, ((ScalarCallable<Integer>)m).call().intValue());
    }
}
