

package io.reactivex.internal.operators.flowable;

import java.util.*;

import io.reactivex.Flowable;
import io.reactivex.internal.util.*;
import io.reactivex.subscribers.DefaultSubscriber;


public final class BlockingFlowableMostRecent<T> implements Iterable<T> {

    final Flowable<T> source;

    final T initialValue;

    public BlockingFlowableMostRecent(Flowable<T> source, T initialValue) {
        this.source = source;
        this.initialValue = initialValue;
    }

    @Override
    public Iterator<T> iterator() {
        MostRecentSubscriber<T> mostRecentSubscriber = new MostRecentSubscriber<T>(initialValue);

        
        source.subscribe(mostRecentSubscriber);

        return mostRecentSubscriber.getIterable();
    }

    static final class MostRecentSubscriber<T> extends DefaultSubscriber<T> {
        volatile Object value;

        MostRecentSubscriber(T value) {
            this.value = NotificationLite.next(value);
        }

        @Override
        public void onComplete() {
            value = NotificationLite.complete();
        }

        @Override
        public void onError(Throwable e) {
            value = NotificationLite.error(e);
        }

        @Override
        public void onNext(T args) {
            value = NotificationLite.next(args);
        }

        
        public Iterator getIterable() {
            return new Iterator();
        }

        final class Iterator implements java.util.Iterator<T> {
            
            private Object buf;

            @Override
            public boolean hasNext() {
                buf = value;
                return !NotificationLite.isComplete(buf);
            }

            @Override
            public T next() {
                try {
                                        if (buf == null) {
                        buf = value;
                    }
                    if (NotificationLite.isComplete(buf)) {
                        throw new NoSuchElementException();
                    }
                    if (NotificationLite.isError(buf)) {
                        throw ExceptionHelper.wrapOrThrow(NotificationLite.getError(buf));
                    }
                    return NotificationLite.getValue(buf);
                }
                finally {
                    buf = null;
                }
            }

            @Override
            public void remove() {
                throw new UnsupportedOperationException("Read only iterator");
            }
        }
    }
}
