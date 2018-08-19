

package io.reactivex.observable;

import static org.junit.Assert.assertEquals;

import java.util.*;

import org.junit.Test;

import io.reactivex.Observable;

public class ObservableStartWithTests {

    @Test
    public void startWith1() {
        List<String> values = Observable.just("one", "two")
                .startWithArray("zero").toList().blockingGet();

        assertEquals("zero", values.get(0));
        assertEquals("two", values.get(2));
    }

    @Test
    public void startWithIterable() {
        List<String> li = new ArrayList<String>();
        li.add("alpha");
        li.add("beta");
        List<String> values = Observable.just("one", "two").startWith(li).toList().blockingGet();

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
        List<String> values = Observable.just("one", "two")
                .startWith(Observable.fromIterable(li))
                .toList()
                .blockingGet();

        assertEquals("alpha", values.get(0));
        assertEquals("beta", values.get(1));
        assertEquals("one", values.get(2));
        assertEquals("two", values.get(3));
    }

    @Test
    public void startWithEmpty() {
        Observable.just(1).startWithArray().test().assertResult(1);
    }
}
