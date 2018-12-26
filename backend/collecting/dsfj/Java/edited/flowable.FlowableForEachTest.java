

package io.reactivex.internal.operators.flowable;

import static org.junit.Assert.assertEquals;

import java.util.*;

import org.junit.Test;

import io.reactivex.Flowable;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.*;

public class FlowableForEachTest {

    @Test
    public void forEachWile() {
        final List<Object> list = new ArrayList<Object>();

        Flowable.range(1, 5)
        .doOnNext(new Consumer<Integer>() {
            @Override
            public void accept(Integer v) throws Exception {
                list.add(v);
            }
        })
        .forEachWhile(new Predicate<Integer>() {
            @Override
            public boolean test(Integer v) throws Exception {
                return v < 3;
            }
        });

        assertEquals(Arrays.asList(1, 2, 3), list);
    }

    @Test
    public void forEachWileWithError() {
        final List<Object> list = new ArrayList<Object>();

        Flowable.range(1, 5).concatWith(Flowable.<Integer>error(new TestException()))
        .doOnNext(new Consumer<Integer>() {
            @Override
            public void accept(Integer v) throws Exception {
                list.add(v);
            }
        })
        .forEachWhile(new Predicate<Integer>() {
            @Override
            public boolean test(Integer v) throws Exception {
                return true;
            }
        }, new Consumer<Throwable>() {
            @Override
            public void accept(Throwable e) throws Exception {
                list.add(100);
            }
        });

        assertEquals(Arrays.asList(1, 2, 3, 4, 5, 100), list);
    }

}
