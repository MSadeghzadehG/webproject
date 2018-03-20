

package io.reactivex.internal.operators.flowable;

import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

import org.reactivestreams.Publisher;

import io.reactivex.*;
import io.reactivex.internal.util.*;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.subscribers.DisposableSubscriber;


public final class BlockingFlowableNext<T> implements Iterable<T> {

    final Publisher<? extends T> source;

    public BlockingFlowableNext(Publisher<? extends T> source) {
        this.source = source;
    }

    @Override
    public Iterator<T> iterator() {
        NextSubscriber<T> nextSubscriber = new NextSubscriber<T>();
        return new NextIterator<T>(source, nextSubscriber);
    }

        static final class NextIterator<T> implements Iterator<T> {

        private final NextSubscriber<T> observer;
        private final Publisher<? extends T> items;
        private T next;
        private boolean hasNext = true;
        private boolean isNextConsumed = true;
        private Throwable error;
        private boolean started;

        NextIterator(Publisher<? extends T> items, NextSubscriber<T> observer) {
            this.items = items;
            this.observer = observer;
        }

        @Override
        public boolean hasNext() {
            if (error != null) {
                                throw ExceptionHelper.wrapOrThrow(error);
            }
                                    if (!hasNext) {
                                return false;
            }
                        return !isNextConsumed || moveToNext();
        }

        private boolean moveToNext() {
            try {
                if (!started) {
                    started = true;
                                        observer.setWaiting();
                    Flowable.<T>fromPublisher(items)
                    .materialize().subscribe(observer);
                }

                Notification<T> nextNotification = observer.takeNext();
                if (nextNotification.isOnNext()) {
                    isNextConsumed = false;
                    next = nextNotification.getValue();
                    return true;
                }
                                                hasNext = false;
                if (nextNotification.isOnComplete()) {
                    return false;
                }
                if (nextNotification.isOnError()) {
                    error = nextNotification.getError();
                    throw ExceptionHelper.wrapOrThrow(error);
                }
                throw new IllegalStateException("Should not reach here");
            } catch (InterruptedException e) {
                observer.dispose();
                error = e;
                throw ExceptionHelper.wrapOrThrow(e);
            }
        }

        @Override
        public T next() {
            if (error != null) {
                                throw ExceptionHelper.wrapOrThrow(error);
            }
            if (hasNext()) {
                isNextConsumed = true;
                return next;
            }
            else {
                throw new NoSuchElementException("No more elements");
            }
        }

        @Override
        public void remove() {
            throw new UnsupportedOperationException("Read only iterator");
        }
    }

    static final class NextSubscriber<T> extends DisposableSubscriber<Notification<T>> {
        private final BlockingQueue<Notification<T>> buf = new ArrayBlockingQueue<Notification<T>>(1);
        final AtomicInteger waiting = new AtomicInteger();

        @Override
        public void onComplete() {
                    }

        @Override
        public void onError(Throwable e) {
            RxJavaPlugins.onError(e);
        }

        @Override
        public void onNext(Notification<T> args) {

            if (waiting.getAndSet(0) == 1 || !args.isOnNext()) {
                Notification<T> toOffer = args;
                while (!buf.offer(toOffer)) {
                    Notification<T> concurrentItem = buf.poll();

                                        if (concurrentItem != null && !concurrentItem.isOnNext()) {
                        toOffer = concurrentItem;
                    }
                }
            }

        }

        public Notification<T> takeNext() throws InterruptedException {
            setWaiting();
            BlockingHelper.verifyNonBlocking();
            return buf.take();
        }
        void setWaiting() {
            waiting.set(1);
        }
    }
}
