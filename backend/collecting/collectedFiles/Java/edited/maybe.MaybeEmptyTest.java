

package io.reactivex.internal.operators.maybe;

import static org.junit.Assert.*;

import org.junit.Test;

import io.reactivex.Maybe;
import io.reactivex.internal.fuseable.ScalarCallable;

public class MaybeEmptyTest {

    @Test
    public void scalarCallable() {
        Maybe<Integer> m = Maybe.empty();

        assertTrue(m.getClass().toString(), m instanceof ScalarCallable);

        assertNull(((ScalarCallable<?>)m).call());
    }
}
