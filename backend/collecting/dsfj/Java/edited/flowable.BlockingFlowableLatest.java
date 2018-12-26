

package io.reactivex.internal.operators.flowable;

import java.util.*;
import java.util.concurrent.Semaphore;
import java.util.concurrent.atomic.AtomicReference;

import org.reactivestreams.Publisher;

import io.reactivex.*;
import io.reactivex.internal.util.*;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.subscribers.DisposableSubscriber;


public final class BlockingFlowableLatest<T> implements Iterable<T> {

    final Publisher<? extends T> source;

    public BlockingFlowableLatest(Publisher<? extends T> source) {
        this.source = source;
    }

    @Override
    public Iterator<T> iterator() {
        LatestSubscriberIterator<T> lio = new LatestSubscriberIterator<T>();
        Flowable.<T>fromPublisher(source).materialize().subscribe(lio);
        return lio;
    }

    
    static final class LatestSubscriberIterator<T> extends DisposableSubscriber<Notification<T>> implements Iterator<T> {
        final Semaphore notify = new Semaphore(0);
                final AtomicReference<Notification<T>> value = new AtomicReference<Notification<T>>();

                Notification<T> iteratorNotification;

        @Override
        public void onNext(Notification<T> args) {
            boolean wasNotAvailable = value.getAndSet(args) == null;
            if (wasNotAvailable) {
                notify.release();
            }
        }

        @Override
        public void onError(Throwable e) {
            RxJavaPlugins.onError(e);
        }

        @Override
        public void onComplete() {
                    }

        @Override
        public boolean hasNext() {
            if (iteratorNotification != null && iteratorNotification.isOnError()) {
                throw ExceptionHelper.wrapOrThrow(iteratorNotification.getError());
            }
            if (iteratorNotification == null || iteratorNotification.isOnNext()) {
                if (iteratorNotification == null) {
                    try {
                        BlockingHelper.verifyNonBlocking();
                        notify.acquire();
                    } catch (InterruptedException ex) {
                        dispose();
                        iteratorNotification = Notification.createOnError(ex);
                        throw ExceptionHelper.wrapOrThrow(ex);
                    }

                    Notification<T> n = value.getAndSet(null);
                    iteratorNotification = n;
                    if (n.isOnError()) {
                        throw ExceptionHelper.wrapOrThrow(n.getError());
                    }
                }
            }
            return iteratorNotification.isOnNext();
        }

        @Override
        public T next() {
            if (hasNext()) {
                if (iteratorNotification.isOnNext()) {
                    T v = iteratorNotification.getValue();
                    iteratorNotification = null;
                    return v;
                }
            }
            throw new NoSuchElementException();
        }

        @Override
        public void remove() {
            throw new UnsupportedOperationException("Read-only iterator.");
        }

    }
}
