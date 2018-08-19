
package io.reactivex.internal.functions;

import io.reactivex.functions.BiPredicate;


public final class ObjectHelper {

    
    private ObjectHelper() {
        throw new IllegalStateException("No instances!");
    }

    
    public static <T> T requireNonNull(T object, String message) {
        if (object == null) {
            throw new NullPointerException(message);
        }
        return object;
    }

    
    public static boolean equals(Object o1, Object o2) {         return o1 == o2 || (o1 != null && o1.equals(o2));
    }

    
    public static int hashCode(Object o) {
        return o != null ? o.hashCode() : 0;
    }

    
    public static int compare(int v1, int v2) {
        return v1 < v2 ? -1 : (v1 > v2 ? 1 : 0);
    }

    
    public static int compare(long v1, long v2) {
        return v1 < v2 ? -1 : (v1 > v2 ? 1 : 0);
    }

    static final BiPredicate<Object, Object> EQUALS = new BiObjectPredicate();

    
    @SuppressWarnings("unchecked")
    public static <T> BiPredicate<T, T> equalsPredicate() {
        return (BiPredicate<T, T>)EQUALS;
    }

    
    public static int verifyPositive(int value, String paramName) {
        if (value <= 0) {
            throw new IllegalArgumentException(paramName + " > 0 required but it was " + value);
        }
        return value;
    }

    
    public static long verifyPositive(long value, String paramName) {
        if (value <= 0L) {
            throw new IllegalArgumentException(paramName + " > 0 required but it was " + value);
        }
        return value;
    }

    static final class BiObjectPredicate implements BiPredicate<Object, Object> {
        @Override
        public boolean test(Object o1, Object o2) {
            return ObjectHelper.equals(o1, o2);
        }
    }

    
    @Deprecated
    public static long requireNonNull(long value, String message) {
        throw new InternalError("Null check on a primitive: " + message);
    }
}
