



package io.reactivex.internal.util;

public final class Pow2 {
    private Pow2() {
        throw new IllegalStateException("No instances!");
    }

    
    public static int roundToPowerOfTwo(final int value) {
        return 1 << (32 - Integer.numberOfLeadingZeros(value - 1));
    }

    
    public static boolean isPowerOfTwo(final int value) {
        return (value & (value - 1)) == 0;
    }
}
