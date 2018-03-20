

package io.reactivex;

import java.util.Iterator;

import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.infra.Blackhole;
import org.reactivestreams.*;

import io.reactivex.internal.subscriptions.EmptySubscription;
import io.reactivex.subscribers.DefaultSubscriber;


public abstract class InputWithIncrementingInteger {
    final class DefaultSubscriberImpl extends DefaultSubscriber<Integer> {
        @Override
        public void onComplete() {

        }

        @Override
        public void onError(Throwable e) {

        }

        @Override
        public void onNext(Integer t) {
            bh.consume(t);
        }
    }

    final class IncrementingIterable implements Iterable<Integer> {
        private final class IncrementingIterator implements Iterator<Integer> {
            int i;

            @Override
            public boolean hasNext() {
                return i < size;
            }

            @Override
            public Integer next() {
                Blackhole.consumeCPU(10);
                return i++;
            }

            @Override
            public void remove() {

            }
        }

        private final int size;

        private IncrementingIterable(int size) {
            this.size = size;
        }

        @Override
        public Iterator<Integer> iterator() {
            return new IncrementingIterator();
        }
    }

    final class IncrementingPublisher implements Publisher<Integer> {
        private final int size;

        IncrementingPublisher(int size) {
            this.size = size;
        }

        @Override
        public void subscribe(Subscriber<? super Integer> s) {
            s.onSubscribe(EmptySubscription.INSTANCE);
            for (int i = 0; i < size; i++) {
                s.onNext(i);
            }
            s.onComplete();
        }
    }

    public Iterable<Integer> iterable;
    public Flowable<Integer> observable;
    public Flowable<Integer> firehose;
    public Blackhole bh;

    public abstract int getSize();

    @Setup
    public void setup(final Blackhole bh) {
        this.bh = bh;
        final int size = getSize();
        observable = Flowable.range(0, size);

        firehose = Flowable.unsafeCreate(new IncrementingPublisher(size));
        iterable = new IncrementingIterable(size);

    }

    public PerfSubscriber newLatchedObserver() {
        return new PerfSubscriber(bh);
    }

    public FlowableSubscriber<Integer> newSubscriber() {
        return new DefaultSubscriberImpl();
    }

}
