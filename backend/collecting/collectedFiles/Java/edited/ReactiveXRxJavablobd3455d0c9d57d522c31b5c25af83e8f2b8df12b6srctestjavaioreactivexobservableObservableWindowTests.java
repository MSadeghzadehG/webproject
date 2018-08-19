

package io.reactivex.observable;

import static org.junit.Assert.*;

import java.util.*;

import org.junit.Test;

import io.reactivex.Observable;
import io.reactivex.functions.*;

public class ObservableWindowTests {

    @Test
    public void testWindow() {
        final ArrayList<List<Integer>> lists = new ArrayList<List<Integer>>();

        Observable.concat(
            Observable.just(1, 2, 3, 4, 5, 6)
            .window(3)
            .map(new Function<Observable<Integer>, Observable<List<Integer>>>() {
                @Override
                public Observable<List<Integer>> apply(Observable<Integer> xs) {
                    return xs.toList().toObservable();
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
