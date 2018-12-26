

package io.reactivex.flowable;

import java.util.*;

import io.reactivex.*;
import io.reactivex.functions.Consumer;
import io.reactivex.schedulers.Schedulers;


public final class FlowableEventStream {
    private FlowableEventStream() {
        throw new IllegalStateException("No instances!");
    }
    public static Flowable<Event> getEventStream(final String type, final int numInstances) {

        return Flowable.<Event>generate(new EventConsumer(type, numInstances))
                .subscribeOn(Schedulers.newThread());
    }

    public static Event randomEvent(String type, int numInstances) {
        Map<String, Object> values = new LinkedHashMap<String, Object>();
        values.put("count200", randomIntFrom0to(4000));
        values.put("count4xx", randomIntFrom0to(300));
        values.put("count5xx", randomIntFrom0to(500));
        return new Event(type, "instance_" + randomIntFrom0to(numInstances), values);
    }

    private static int randomIntFrom0to(int max) {
                long x = System.nanoTime();
        x ^= (x << 21);
        x ^= (x >>> 35);
        x ^= (x << 4);
        return Math.abs((int) x % max);
    }

    static final class EventConsumer implements Consumer<Emitter<Event>> {
        private final String type;
        private final int numInstances;

        EventConsumer(String type, int numInstances) {
            this.type = type;
            this.numInstances = numInstances;
        }

        @Override
        public void accept(Emitter<Event> s) {
            s.onNext(randomEvent(type, numInstances));
            try {
                                Thread.sleep(50);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                s.onError(e);
            }
        }
    }

    public static class Event {
        public final String type;
        public final String instanceId;
        public final Map<String, Object> values;

        
        public Event(String type, String instanceId, Map<String, Object> values) {
            this.type = type;
            this.instanceId = instanceId;
            this.values = Collections.unmodifiableMap(values);
        }
    }
}
