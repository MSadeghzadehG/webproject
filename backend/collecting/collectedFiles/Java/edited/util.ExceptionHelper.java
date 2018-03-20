

package io.reactivex.internal.util;

import java.util.*;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.exceptions.CompositeException;


public final class ExceptionHelper {

    
    private ExceptionHelper() {
        throw new IllegalStateException("No instances!");
    }

    
    public static RuntimeException wrapOrThrow(Throwable error) {
        if (error instanceof Error) {
            throw (Error)error;
        }
        if (error instanceof RuntimeException) {
            return (RuntimeException)error;
        }
        return new RuntimeException(error);
    }

    
    public static final Throwable TERMINATED = new Termination();

    public static <T> boolean addThrowable(AtomicReference<Throwable> field, Throwable exception) {
        for (;;) {
            Throwable current = field.get();

            if (current == TERMINATED) {
                return false;
            }

            Throwable update;
            if (current == null) {
                update = exception;
            } else {
                update = new CompositeException(current, exception);
            }

            if (field.compareAndSet(current, update)) {
                return true;
            }
        }
    }

    public static <T> Throwable terminate(AtomicReference<Throwable> field) {
        Throwable current = field.get();
        if (current != TERMINATED) {
            current = field.getAndSet(TERMINATED);
        }
        return current;
    }

    
    public static List<Throwable> flatten(Throwable t) {
        List<Throwable> list = new ArrayList<Throwable>();
        ArrayDeque<Throwable> deque = new ArrayDeque<Throwable>();
        deque.offer(t);

        while (!deque.isEmpty()) {
            Throwable e = deque.removeFirst();
            if (e instanceof CompositeException) {
                CompositeException ce = (CompositeException) e;
                List<Throwable> exceptions = ce.getExceptions();
                for (int i = exceptions.size() - 1; i >= 0; i--) {
                    deque.offerFirst(exceptions.get(i));
                }
            } else {
                list.add(e);
            }
        }

        return list;
    }

    
    @SuppressWarnings("unchecked")
    public static <E extends Throwable> Exception throwIfThrowable(Throwable e) throws E {
        if (e instanceof Exception) {
            return (Exception)e;
        }
        throw (E)e;
    }

    static final class Termination extends Throwable {

        private static final long serialVersionUID = -4649703670690200604L;

        Termination() {
            super("No further exceptions");
        }

        @Override
        public Throwable fillInStackTrace() {
            return this;
        }
    }
}
