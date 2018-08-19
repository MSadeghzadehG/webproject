

package io.reactivex.internal.operators.observable;

import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.*;
import io.reactivex.internal.util.*;
import io.reactivex.observers.DisposableObserver;
import io.reactivex.plugins.RxJavaPlugins;


public final class BlockingObservableNext<T> implements Iterable<T> {

    final ObservableSource<T> source;

    public BlockingObservableNext(ObservableSource<T> source) {
        this.source = source;
    }

    @Override
    public Iterator<T> iterator() {
        NextObserver<T> nextObserver = new NextObserver<T>();
        return new NextIterator<T>(source, nextObserver);
    }

        static final class NextIterator<T> implements Iterator<T> {

        private final NextObserver<T> observer;
        private final ObservableSource<T> items;
        private T next;
        private boolean hasNext = true;
        private boolean isNextConsumed = true;
        private Throwable error;
        private boolean started;

        NextIterator(ObservableSource<T> items, NextObserver<T> observer) {
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
            if (!started) {
                started = true;
                                observer.setWaiting();
                new ObservableMaterialize<T>(items).subscribe(observer);
            }

            Notification<T> nextNotification;

            try {
                nextNotification = observer.takeNext();
            } catch (InterruptedException e) {
                observer.dispose();
                error = e;
                throw ExceptionHelper.wrapOrThrow(e);
            }

            if (nextNotification.isOnNext()) {
                isNextConsumed = false;
                next = nextNotification.getValue();
                return true;
            }
                                    hasNext = false;
            if (nextNotification.isOnComplete()) {
                return false;
            }
            error = nextNotification.getError();
            throw ExceptionHelper.wrapOrThrow(error);
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

    static final class NextObserver<T> extends DisposableObserver<Notification<T>> {
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
