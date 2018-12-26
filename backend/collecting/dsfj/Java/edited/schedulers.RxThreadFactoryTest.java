

package io.reactivex.internal.schedulers;

import static org.junit.Assert.*;
import org.junit.Test;

import io.reactivex.internal.functions.Functions;

public class RxThreadFactoryTest {

    @Test
    public void normal() {
        RxThreadFactory tf = new RxThreadFactory("Test", 1);

        assertEquals("RxThreadFactory[Test]", tf.toString());

        Thread t = tf.newThread(Functions.EMPTY_RUNNABLE);

        assertTrue(t.isDaemon());
        assertEquals(1, t.getPriority());
    }
}
