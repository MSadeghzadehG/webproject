

package com.google.common.primitives;

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.base.Preconditions.checkPositionIndexes;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.annotations.VisibleForTesting;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.Comparator;
import sun.misc.Unsafe;


@GwtIncompatible
public final class UnsignedBytes {
  private UnsignedBytes() {}

  
  public static final byte MAX_POWER_OF_TWO = (byte) 0x80;

  
  public static final byte MAX_VALUE = (byte) 0xFF;

  private static final int UNSIGNED_MASK = 0xFF;

  
  public static int toInt(byte value) {
    return value & UNSIGNED_MASK;
  }

  
  @CanIgnoreReturnValue
  public static byte checkedCast(long value) {
    checkArgument(value >> Byte.SIZE == 0, "out of range: %s", value);
    return (byte) value;
  }

  
  public static byte saturatedCast(long value) {
    if (value > toInt(MAX_VALUE)) {
      return MAX_VALUE;     }
    if (value < 0) {
      return (byte) 0;
    }
    return (byte) value;
  }

  
  public static int compare(byte a, byte b) {
    return toInt(a) - toInt(b);
  }

  
  public static byte min(byte... array) {
    checkArgument(array.length > 0);
    int min = toInt(array[0]);
    for (int i = 1; i < array.length; i++) {
      int next = toInt(array[i]);
      if (next < min) {
        min = next;
      }
    }
    return (byte) min;
  }

  
  public static byte max(byte... array) {
    checkArgument(array.length > 0);
    int max = toInt(array[0]);
    for (int i = 1; i < array.length; i++) {
      int next = toInt(array[i]);
      if (next > max) {
        max = next;
      }
    }
    return (byte) max;
  }

  
  @Beta
  public static String toString(byte x) {
    return toString(x, 10);
  }

  
  @Beta
  public static String toString(byte x, int radix) {
    checkArgument(
        radix >= Character.MIN_RADIX && radix <= Character.MAX_RADIX,
        "radix (%s) must be between Character.MIN_RADIX and Character.MAX_RADIX",
        radix);
        return Integer.toString(toInt(x), radix);
  }

  
  @Beta
  @CanIgnoreReturnValue
  public static byte parseUnsignedByte(String string) {
    return parseUnsignedByte(string, 10);
  }

  
  @Beta
  @CanIgnoreReturnValue
  public static byte parseUnsignedByte(String string, int radix) {
    int parse = Integer.parseInt(checkNotNull(string), radix);
        if (parse >> Byte.SIZE == 0) {
      return (byte) parse;
    } else {
      throw new NumberFormatException("out of range: " + parse);
    }
  }

  
  public static String join(String separator, byte... array) {
    checkNotNull(separator);
    if (array.length == 0) {
      return "";
    }

        StringBuilder builder = new StringBuilder(array.length * (3 + separator.length()));
    builder.append(toInt(array[0]));
    for (int i = 1; i < array.length; i++) {
      builder.append(separator).append(toString(array[i]));
    }
    return builder.toString();
  }

  
  public static Comparator<byte[]> lexicographicalComparator() {
    return LexicographicalComparatorHolder.BEST_COMPARATOR;
  }

  @VisibleForTesting
  static Comparator<byte[]> lexicographicalComparatorJavaImpl() {
    return LexicographicalComparatorHolder.PureJavaComparator.INSTANCE;
  }

  
  @VisibleForTesting
  static class LexicographicalComparatorHolder {
    static final String UNSAFE_COMPARATOR_NAME =
        LexicographicalComparatorHolder.class.getName() + "$UnsafeComparator";

    static final Comparator<byte[]> BEST_COMPARATOR = getBestComparator();

    @VisibleForTesting
    enum UnsafeComparator implements Comparator<byte[]> {
      INSTANCE;

      static final boolean BIG_ENDIAN = ByteOrder.nativeOrder().equals(ByteOrder.BIG_ENDIAN);

      

      static final Unsafe theUnsafe = getUnsafe();

      
      static final int BYTE_ARRAY_BASE_OFFSET = theUnsafe.arrayBaseOffset(byte[].class);

      static {
                        if (!("64".equals(System.getProperty("sun.arch.data.model"))
            && (BYTE_ARRAY_BASE_OFFSET % 8) == 0
                        && theUnsafe.arrayIndexScale(byte[].class) == 1)) {
          throw new Error();         }
      }

      
      private static sun.misc.Unsafe getUnsafe() {
        try {
          return sun.misc.Unsafe.getUnsafe();
        } catch (SecurityException e) {
                  }
        try {
          return java.security.AccessController.doPrivileged(
              new java.security.PrivilegedExceptionAction<sun.misc.Unsafe>() {
                @Override
                public sun.misc.Unsafe run() throws Exception {
                  Class<sun.misc.Unsafe> k = sun.misc.Unsafe.class;
                  for (java.lang.reflect.Field f : k.getDeclaredFields()) {
                    f.setAccessible(true);
                    Object x = f.get(null);
                    if (k.isInstance(x)) {
                      return k.cast(x);
                    }
                  }
                  throw new NoSuchFieldError("the Unsafe");
                }
              });
        } catch (java.security.PrivilegedActionException e) {
          throw new RuntimeException("Could not initialize intrinsics", e.getCause());
        }
      }

      @Override
      public int compare(byte[] left, byte[] right) {
        final int stride = 8;
        int minLength = Math.min(left.length, right.length);
        int strideLimit = minLength & ~(stride - 1);
        int i;

        
        for (i = 0; i < strideLimit; i += stride) {
          long lw = theUnsafe.getLong(left, BYTE_ARRAY_BASE_OFFSET + (long) i);
          long rw = theUnsafe.getLong(right, BYTE_ARRAY_BASE_OFFSET + (long) i);
          if (lw != rw) {
            if (BIG_ENDIAN) {
              return UnsignedLongs.compare(lw, rw);
            }

            
            int n = Long.numberOfTrailingZeros(lw ^ rw) & ~0x7;
            return ((int) ((lw >>> n) & UNSIGNED_MASK)) - ((int) ((rw >>> n) & UNSIGNED_MASK));
          }
        }

                for (; i < minLength; i++) {
          int result = UnsignedBytes.compare(left[i], right[i]);
          if (result != 0) {
            return result;
          }
        }
        return left.length - right.length;
      }

      @Override
      public String toString() {
        return "UnsignedBytes.lexicographicalComparator() (sun.misc.Unsafe version)";
      }
    }

    enum PureJavaComparator implements Comparator<byte[]> {
      INSTANCE;

      @Override
      public int compare(byte[] left, byte[] right) {
        int minLength = Math.min(left.length, right.length);
        for (int i = 0; i < minLength; i++) {
          int result = UnsignedBytes.compare(left[i], right[i]);
          if (result != 0) {
            return result;
          }
        }
        return left.length - right.length;
      }

      @Override
      public String toString() {
        return "UnsignedBytes.lexicographicalComparator() (pure Java version)";
      }
    }

    
    static Comparator<byte[]> getBestComparator() {
      try {
        Class<?> theClass = Class.forName(UNSAFE_COMPARATOR_NAME);

                @SuppressWarnings("unchecked")
        Comparator<byte[]> comparator = (Comparator<byte[]>) theClass.getEnumConstants()[0];
        return comparator;
      } catch (Throwable t) {         return lexicographicalComparatorJavaImpl();
      }
    }
  }

  private static byte flip(byte b) {
    return (byte) (b ^ 0x80);
  }

  
  public static void sort(byte[] array) {
    checkNotNull(array);
    sort(array, 0, array.length);
  }

  
  public static void sort(byte[] array, int fromIndex, int toIndex) {
    checkNotNull(array);
    checkPositionIndexes(fromIndex, toIndex, array.length);
    for (int i = fromIndex; i < toIndex; i++) {
      array[i] = flip(array[i]);
    }
    Arrays.sort(array, fromIndex, toIndex);
    for (int i = fromIndex; i < toIndex; i++) {
      array[i] = flip(array[i]);
    }
  }

  
  public static void sortDescending(byte[] array) {
    checkNotNull(array);
    sortDescending(array, 0, array.length);
  }

  
  public static void sortDescending(byte[] array, int fromIndex, int toIndex) {
    checkNotNull(array);
    checkPositionIndexes(fromIndex, toIndex, array.length);
    for (int i = fromIndex; i < toIndex; i++) {
      array[i] ^= Byte.MAX_VALUE;
    }
    Arrays.sort(array, fromIndex, toIndex);
    for (int i = fromIndex; i < toIndex; i++) {
      array[i] ^= Byte.MAX_VALUE;
    }
  }
}
