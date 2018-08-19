

package io.reactivex.flowable;

import static org.junit.Assert.*;

import java.util.*;

import org.junit.Test;

import io.reactivex.Flowable;
import io.reactivex.functions.*;

public class FlowableWindowTests {

    @Test
    public void testWindow() {
        final ArrayList<List<Integer>> lists = new ArrayList<List<Integer>>();

        Flowable.concat(
            Flowable.just(1, 2, 3, 4, 5, 6)
            .window(3)
            .map(new Function<Flowable<Integer>, Flowable<List<Integer>>>() {
                @Override
                public Flowable<List<Integer>> apply(Flowable<Integer> xs) {
                    return xs.toList().toFlowable();
                }
            })
        )
        .blockingForEach(new Consumer<List<Integer>>() {
            @Override
            public void accept(List<Integer> xs) {
                lists.add(xs);
            }
        });

        assertArrayEquals(lists.get(0).toArray(new Integer[3]), new Integer[] { 1, 2, 3 });
        assertArrayEquals(lists.get(1).toArray(new Integer[3]), new Integer[] { 4, 5, 6 });
        assertEquals(2, lists.size());

    }
}
