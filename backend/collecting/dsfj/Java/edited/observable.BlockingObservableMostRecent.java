

package io.reactivex.internal.operators.observable;


import java.util.*;

import io.reactivex.ObservableSource;
import io.reactivex.internal.util.*;
import io.reactivex.observers.DefaultObserver;


public final class BlockingObservableMostRecent<T> implements Iterable<T> {

    final ObservableSource<T> source;

    final T initialValue;

    public BlockingObservableMostRecent(ObservableSource<T> source, T initialValue) {
        this.source = source;
        this.initialValue = initialValue;
    }

    @Override
    public Iterator<T> iterator() {
        MostRecentObserver<T> mostRecentObserver = new MostRecentObserver<T>(initialValue);

        
        source.subscribe(mostRecentObserver);

        return mostRecentObserver.getIterable();
    }

    static final class MostRecentObserver<T> extends DefaultObserver<T> {
        volatile Object value;

        MostRecentObserver(T value) {
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
