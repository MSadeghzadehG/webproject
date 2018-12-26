

package io.reactivex.tck;

import java.util.*;

import org.reactivestreams.Publisher;
import org.reactivestreams.tck.*;
import org.testng.annotations.Test;

import io.reactivex.Flowable;
import io.reactivex.exceptions.TestException;


@Test
public abstract class BaseTck<T> extends PublisherVerification<T> {

    public BaseTck() {
        this(25L);
    }

    public BaseTck(long timeout) {
        super(new TestEnvironment(timeout));
    }

    @Override
    public Publisher<T> createFailedPublisher() {
        return Flowable.error(new TestException());
    }


    @Override
    public long maxElementsFromPublisher() {
        return 1024;
    }

    
    protected Iterable<Long> iterate(long elements) {
        return iterate(elements > Integer.MAX_VALUE, elements);
    }

    protected Iterable<Long> iterate(boolean useInfinite, long elements) {
        return useInfinite ? new InfiniteRange() : new FiniteRange(elements);
    }

    
    protected Long[] array(long elements) {
        Long[] a = new Long[(int)elements];
        for (int i = 0; i < elements; i++) {
            a[i] = (long)i;
        }
        return a;
    }

    static final class FiniteRange implements Iterable<Long> {
        final long end;
        FiniteRange(long end) {
            this.end = end;
        }

        @Override
        public Iterator<Long> iterator() {
            return new FiniteRangeIterator(end);
        }

        static final class FiniteRangeIterator implements Iterator<Long> {
            final long end;
            long count;

            FiniteRangeIterator(long end) {
                this.end = end;
            }

            @Override
            public boolean hasNext() {
                return count != end;
            }

            @Override
            public Long next() {
                long c = count;
                if (c != end) {
                    count = c + 1;
                    return c;
                }
                throw new NoSuchElementException();
            }

            @Override
            public void remove() {
                throw new UnsupportedOperationException();
            }
        }
    }

    static final class InfiniteRange implements Iterable<Long> {
        @Override
        public Iterator<Long> iterator() {
            return new InfiniteRangeIterator();
        }

        static final class InfiniteRangeIterator implements Iterator<Long> {
            long count;

            @Override
            public boolean hasNext() {
                return true;
            }

            @Override
            public Long next() {
                return count++;
            }

            @Override
            public void remove() {
                throw new UnsupportedOperationException();
            }
        }
    }
}
