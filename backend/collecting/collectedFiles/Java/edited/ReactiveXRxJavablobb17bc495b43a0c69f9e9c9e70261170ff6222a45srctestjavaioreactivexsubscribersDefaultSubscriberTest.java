

package io.reactivex.subscribers;

import static org.junit.Assert.assertEquals;

import java.util.*;

import org.junit.Test;

import io.reactivex.Flowable;

public class DefaultSubscriberTest {

    static final class RequestEarly extends DefaultSubscriber<Integer> {

        final List<Object> events = new ArrayList<Object>();

        RequestEarly() {
            request(5);
        }

        @Override
        protected void onStart() {
        }

        @Override
        public void onNext(Integer t) {
            events.add(t);
        }

        @Override
        public void onError(Throwable t) {
            events.add(t);
        }

        @Override
        public void onComplete() {
            events.add("Done");
        }

    }

    @Test
    public void requestUpfront() {
        RequestEarly sub = new RequestEarly();

        Flowable.range(1, 10).subscribe(sub);

        assertEquals(Collections.emptyList(), sub.events);
    }

}
