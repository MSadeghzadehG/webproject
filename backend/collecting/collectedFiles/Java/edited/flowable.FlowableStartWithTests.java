

package io.reactivex.flowable;

import static org.junit.Assert.assertEquals;

import java.util.*;

import org.junit.Test;

import io.reactivex.Flowable;

public class FlowableStartWithTests {

    @Test
    public void startWith1() {
        List<String> values = Flowable.just("one", "two")
                .startWithArray("zero").toList().blockingGet();

        assertEquals("zero", values.get(0));
        assertEquals("two", values.get(2));
    }

    @Test
    public void startWithIterable() {
        List<String> li = new ArrayList<String>();
        li.add("alpha");
        li.add("beta");
        List<String> values = Flowable.just("one", "two").startWith(li).toList().blockingGet();

        assertEquals("alpha", values.get(0));
        assertEquals("beta", values.get(1));
        assertEquals("one", values.get(2));
        assertEquals("two", values.get(3));
    }

    @Test
    public void startWithObservable() {
        List<String> li = new ArrayList<String>();
        li.add("alpha");
        li.add("beta");
        List<String> values = Flowable.just("one", "two")
                .startWith(Flowable.fromIterable(li))
                .toList()
                .blockingGet();

        assertEquals("alpha", values.get(0));
        assertEquals("beta", values.get(1));
        assertEquals("one", values.get(2));
        assertEquals("two", values.get(3));
    }

    @Test
    public void startWithEmpty() {
        Flowable.just(1).startWithArray().test().assertResult(1);
    }

}
