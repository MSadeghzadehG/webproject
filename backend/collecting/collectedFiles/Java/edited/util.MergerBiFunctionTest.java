

package io.reactivex.internal.util;

import static org.junit.Assert.assertEquals;

import java.util.*;

import org.junit.Test;

public class MergerBiFunctionTest {

    @Test
    public void firstEmpty() throws Exception {
        MergerBiFunction<Integer> merger = new MergerBiFunction<Integer>(new Comparator<Integer>() {
            @Override
            public int compare(Integer o1, Integer o2) {
                return o1.compareTo(o2);
            }
        });
        List<Integer> list = merger.apply(Collections.<Integer>emptyList(), Arrays.asList(3, 5));

        assertEquals(Arrays.asList(3, 5), list);
    }

    @Test
    public void bothEmpty() throws Exception {
        MergerBiFunction<Integer> merger = new MergerBiFunction<Integer>(new Comparator<Integer>() {
            @Override
            public int compare(Integer o1, Integer o2) {
                return o1.compareTo(o2);
            }
        });
        List<Integer> list = merger.apply(Collections.<Integer>emptyList(), Collections.<Integer>emptyList());

        assertEquals(Collections.<Integer>emptyList(), list);
    }

    @Test
    public void secondEmpty() throws Exception {
        MergerBiFunction<Integer> merger = new MergerBiFunction<Integer>(new Comparator<Integer>() {
            @Override
            public int compare(Integer o1, Integer o2) {
                return o1.compareTo(o2);
            }
        });
        List<Integer> list = merger.apply(Arrays.asList(2, 4), Collections.<Integer>emptyList());

        assertEquals(Arrays.asList(2, 4), list);
    }

    @Test
    public void sameSize() throws Exception {
        MergerBiFunction<Integer> merger = new MergerBiFunction<Integer>(new Comparator<Integer>() {
            @Override
            public int compare(Integer o1, Integer o2) {
                return o1.compareTo(o2);
            }
        });
        List<Integer> list = merger.apply(Arrays.asList(2, 4), Arrays.asList(3, 5));

        assertEquals(Arrays.asList(2, 3, 4, 5), list);
    }

    @Test
    public void sameSizeReverse() throws Exception {
        MergerBiFunction<Integer> merger = new MergerBiFunction<Integer>(new Comparator<Integer>() {
            @Override
            public int compare(Integer o1, Integer o2) {
                return o1.compareTo(o2);
            }
        });
        List<Integer> list = merger.apply(Arrays.asList(3, 5), Arrays.asList(2, 4));

        assertEquals(Arrays.asList(2, 3, 4, 5), list);
    }
}
