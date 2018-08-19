

package io.reactivex.internal.util;

import java.util.Iterator;

import io.reactivex.exceptions.TestException;


public final class CrashingIterable implements Iterable<Integer> {
    int crashOnIterator;

    final int crashOnHasNext;

    final int crashOnNext;

    public CrashingIterable(int crashOnIterator, int crashOnHasNext, int crashOnNext) {
        this.crashOnIterator = crashOnIterator;
        this.crashOnHasNext = crashOnHasNext;
        this.crashOnNext = crashOnNext;
    }

    @Override
    public Iterator<Integer> iterator() {
        if (--crashOnIterator <= 0) {
            throw new TestException("iterator()");
        }
        return new CrashingIterator(crashOnHasNext, crashOnNext);
    }

    static final class CrashingIterator implements Iterator<Integer> {
        int crashOnHasNext;

        int crashOnNext;

        int count;

        CrashingIterator(int crashOnHasNext, int crashOnNext) {
            this.crashOnHasNext = crashOnHasNext;
            this.crashOnNext = crashOnNext;
        }

        @Override
        public boolean hasNext() {
            if (--crashOnHasNext <= 0) {
                throw new TestException("hasNext()");
            }
            return true;
        }

        @Override
        public Integer next() {
            if (--crashOnNext <= 0) {
                throw new TestException("next()");
            }
            return count++;
        }

        @Override
        public void remove() {
            throw new UnsupportedOperationException();
        }
    }
}
