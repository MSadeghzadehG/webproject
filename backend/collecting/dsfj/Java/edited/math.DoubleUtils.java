

package com.google.common.math;

import static com.google.common.base.Preconditions.checkArgument;
import static java.lang.Double.MAX_EXPONENT;
import static java.lang.Double.MIN_EXPONENT;
import static java.lang.Double.POSITIVE_INFINITY;
import static java.lang.Double.doubleToRawLongBits;
import static java.lang.Double.isNaN;
import static java.lang.Double.longBitsToDouble;
import static java.lang.Math.getExponent;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.annotations.VisibleForTesting;
import java.math.BigInteger;


@GwtIncompatible
final class DoubleUtils {
  private DoubleUtils() {}

  static double nextDown(double d) {
    return -Math.nextUp(-d);
  }

      static final long SIGNIFICAND_MASK = 0x000fffffffffffffL;

      static final long EXPONENT_MASK = 0x7ff0000000000000L;

      static final long SIGN_MASK = 0x8000000000000000L;

  static final int SIGNIFICAND_BITS = 52;

  static final int EXPONENT_BIAS = 1023;

  
  static final long IMPLICIT_BIT = SIGNIFICAND_MASK + 1;

  static long getSignificand(double d) {
    checkArgument(isFinite(d), "not a normal value");
    int exponent = getExponent(d);
    long bits = doubleToRawLongBits(d);
    bits &= SIGNIFICAND_MASK;
    return (exponent == MIN_EXPONENT - 1) ? bits << 1 : bits | IMPLICIT_BIT;
  }

  static boolean isFinite(double d) {
    return getExponent(d) <= MAX_EXPONENT;
  }

  static boolean isNormal(double d) {
    return getExponent(d) >= MIN_EXPONENT;
  }

  
  static double scaleNormalize(double x) {
    long significand = doubleToRawLongBits(x) & SIGNIFICAND_MASK;
    return longBitsToDouble(significand | ONE_BITS);
  }

  static double bigToDouble(BigInteger x) {
        BigInteger absX = x.abs();
    int exponent = absX.bitLength() - 1;
        if (exponent < Long.SIZE - 1) {
      return x.longValue();
    } else if (exponent > MAX_EXPONENT) {
      return x.signum() * POSITIVE_INFINITY;
    }

    
    int shift = exponent - SIGNIFICAND_BITS - 1;
    long twiceSignifFloor = absX.shiftRight(shift).longValue();
    long signifFloor = twiceSignifFloor >> 1;
    signifFloor &= SIGNIFICAND_MASK; 
    
    boolean increment =
        (twiceSignifFloor & 1) != 0 && ((signifFloor & 1) != 0 || absX.getLowestSetBit() < shift);
    long signifRounded = increment ? signifFloor + 1 : signifFloor;
    long bits = (long) ((exponent + EXPONENT_BIAS)) << SIGNIFICAND_BITS;
    bits += signifRounded;
    
    bits |= x.signum() & SIGN_MASK;
    return longBitsToDouble(bits);
  }

  
  static double ensureNonNegative(double value) {
    checkArgument(!isNaN(value));
    if (value > 0.0) {
      return value;
    } else {
      return 0.0;
    }
  }

  @VisibleForTesting static final long ONE_BITS = 0x3ff0000000000000L;
}
