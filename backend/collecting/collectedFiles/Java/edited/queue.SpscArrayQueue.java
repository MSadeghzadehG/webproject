



package io.reactivex.internal.queue;

import java.util.concurrent.atomic.*;

import io.reactivex.annotations.Nullable;
import io.reactivex.internal.fuseable.SimplePlainQueue;
import io.reactivex.internal.util.Pow2;


public final class SpscArrayQueue<E> extends AtomicReferenceArray<E> implements SimplePlainQueue<E> {
    private static final long serialVersionUID = -1296597691183856449L;
    private static final Integer MAX_LOOK_AHEAD_STEP = Integer.getInteger("jctools.spsc.max.lookahead.step", 4096);
    final int mask;
    final AtomicLong producerIndex;
    long producerLookAhead;
    final AtomicLong consumerIndex;
    final int lookAheadStep;

    public SpscArrayQueue(int capacity) {
        super(Pow2.roundToPowerOfTwo(capacity));
        this.mask = length() - 1;
        this.producerIndex = new AtomicLong();
        this.consumerIndex = new AtomicLong();
        lookAheadStep = Math.min(capacity / 4, MAX_LOOK_AHEAD_STEP);
    }

    @Override
    public boolean offer(E e) {
        if (null == e) {
            throw new NullPointerException("Null is not a valid element");
        }
                final int mask = this.mask;
        final long index = producerIndex.get();
        final int offset = calcElementOffset(index, mask);
        if (index >= producerLookAhead) {
            int step = lookAheadStep;
            if (null == lvElement(calcElementOffset(index + step, mask))) {                 producerLookAhead = index + step;
            } else if (null != lvElement(offset)) {
                return false;
            }
        }
        soElement(offset, e);         soProducerIndex(index + 1);         return true;
    }

    @Override
    public boolean offer(E v1, E v2) {
                return offer(v1) && offer(v2);
    }

    @Nullable
    @Override
    public E poll() {
        final long index = consumerIndex.get();
        final int offset = calcElementOffset(index);
                final E e = lvElement(offset);        if (null == e) {
            return null;
        }
        soConsumerIndex(index + 1);         soElement(offset, null);        return e;
    }

    @Override
    public boolean isEmpty() {
        return producerIndex.get() == consumerIndex.get();
    }

    void soProducerIndex(long newIndex) {
        producerIndex.lazySet(newIndex);
    }

    void soConsumerIndex(long newIndex) {
        consumerIndex.lazySet(newIndex);
    }

    @Override
    public void clear() {
                while (poll() != null || !isEmpty()) { }     }

    int calcElementOffset(long index, int mask) {
        return (int)index & mask;
    }

    int calcElementOffset(long index) {
        return (int)index & mask;
    }

    void soElement(int offset, E value) {
        lazySet(offset, value);
    }

    E lvElement(int offset) {
        return get(offset);
    }
}

