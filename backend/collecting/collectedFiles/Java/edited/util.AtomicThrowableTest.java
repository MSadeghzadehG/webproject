

package io.reactivex.internal.util;

import static org.junit.Assert.*;

import org.junit.Test;

public class AtomicThrowableTest {

    @Test
    public void isTerminated() {
        AtomicThrowable ex = new AtomicThrowable();

        assertFalse(ex.isTerminated());

        assertNull(ex.terminate());

        assertTrue(ex.isTerminated());
    }
}
