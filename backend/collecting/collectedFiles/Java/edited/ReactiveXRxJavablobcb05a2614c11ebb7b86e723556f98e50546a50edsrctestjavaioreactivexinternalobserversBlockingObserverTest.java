

package io.reactivex.internal.observers;

import static org.junit.Assert.*;

import java.util.*;

import org.junit.Test;

public class BlockingObserverTest {

    @Test
    public void dispose() {
        Queue<Object> q = new ArrayDeque<Object>();

        BlockingObserver<Object> bo = new BlockingObserver<Object>(q);

        bo.dispose();

        assertEquals(BlockingObserver.TERMINATED, q.poll());

        bo.dispose();

        assertNull(q.poll());
    }
}
