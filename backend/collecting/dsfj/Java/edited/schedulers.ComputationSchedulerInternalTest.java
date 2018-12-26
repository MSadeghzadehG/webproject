
package io.reactivex.internal.schedulers;

import static org.junit.Assert.*;
import org.junit.Test;

public class ComputationSchedulerInternalTest {

    @Test
    public void capPoolSize() {
        assertEquals(8, ComputationScheduler.cap(8, -1));
        assertEquals(8, ComputationScheduler.cap(8, 0));
        assertEquals(4, ComputationScheduler.cap(8, 4));
        assertEquals(8, ComputationScheduler.cap(8, 8));
        assertEquals(8, ComputationScheduler.cap(8, 9));
        assertEquals(8, ComputationScheduler.cap(8, 16));
    }
}
