



package io.reactivex.internal.queue;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.annotations.Nullable;
import io.reactivex.internal.fuseable.SimplePlainQueue;


public final class MpscLinkedQueue<T> implements SimplePlainQueue<T> {
    private final AtomicReference<LinkedQueueNode<T>> producerNode;
    private final AtomicReference<LinkedQueueNode<T>> consumerNode;

    public MpscLinkedQueue() {
        producerNode = new AtomicReference<LinkedQueueNode<T>>();
        consumerNode = new AtomicReference<LinkedQueueNode<T>>();
        LinkedQueueNode<T> node = new LinkedQueueNode<T>();
        spConsumerNode(node);
        xchgProducerNode(node);    }

    
    @Override
    public boolean offer(final T e) {
        if (null == e) {
            throw new NullPointerException("Null is not a valid element");
        }
        final LinkedQueueNode<T> nextNode = new LinkedQueueNode<T>(e);
        final LinkedQueueNode<T> prevProducerNode = xchgProducerNode(nextNode);
                        prevProducerNode.soNext(nextNode);         return true;
    }

    
    @Nullable
    @Override
    public T poll() {
        LinkedQueueNode<T> currConsumerNode = lpConsumerNode();         LinkedQueueNode<T> nextNode = currConsumerNode.lvNext();
        if (nextNode != null) {
                        final T nextValue = nextNode.getAndNullValue();
            spConsumerNode(nextNode);
            return nextValue;
        }
        else if (currConsumerNode != lvProducerNode()) {
                        while ((nextNode = currConsumerNode.lvNext()) == null) { }             
                        final T nextValue = nextNode.getAndNullValue();
            spConsumerNode(nextNode);
            return nextValue;
        }
        return null;
    }

    @Override
    public boolean offer(T v1, T v2) {
        offer(v1);
        offer(v2);
        return true;
    }

    @Override
    public void clear() {
        while (poll() != null && !isEmpty()) { }     }
    LinkedQueueNode<T> lvProducerNode() {
        return producerNode.get();
    }
    LinkedQueueNode<T> xchgProducerNode(LinkedQueueNode<T> node) {
        return producerNode.getAndSet(node);
    }
    LinkedQueueNode<T> lvConsumerNode() {
        return consumerNode.get();
    }

    LinkedQueueNode<T> lpConsumerNode() {
        return consumerNode.get();
    }
    void spConsumerNode(LinkedQueueNode<T> node) {
        consumerNode.lazySet(node);
    }

    
    @Override
    public boolean isEmpty() {
        return lvConsumerNode() == lvProducerNode();
    }

    static final class LinkedQueueNode<E> extends AtomicReference<LinkedQueueNode<E>> {

        private static final long serialVersionUID = 2404266111789071508L;

        private E value;

        LinkedQueueNode() {
        }

        LinkedQueueNode(E val) {
            spValue(val);
        }
        
        public E getAndNullValue() {
            E temp = lpValue();
            spValue(null);
            return temp;
        }

        public E lpValue() {
            return value;
        }

        public void spValue(E newValue) {
            value = newValue;
        }

        public void soNext(LinkedQueueNode<E> n) {
            lazySet(n);
        }

        public LinkedQueueNode<E> lvNext() {
            return get();
        }
    }
}
