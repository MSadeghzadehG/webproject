

package com.google.common.math;

import static java.math.BigInteger.ONE;
import static java.math.BigInteger.ZERO;
import static java.math.RoundingMode.CEILING;
import static java.math.RoundingMode.DOWN;
import static java.math.RoundingMode.FLOOR;
import static java.math.RoundingMode.HALF_DOWN;
import static java.math.RoundingMode.HALF_EVEN;
import static java.math.RoundingMode.HALF_UP;
import static java.math.RoundingMode.UP;
import static java.util.Arrays.asList;

import com.google.common.annotations.GwtCompatible;
import com.google.common.base.Function;
import com.google.common.base.Predicate;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableSet;
import com.google.common.collect.Iterables;
import com.google.common.primitives.Doubles;
import java.math.BigInteger;
import java.math.RoundingMode;


@GwtCompatible
public class MathTesting {
  static final ImmutableSet<RoundingMode> ALL_ROUNDING_MODES =
      ImmutableSet.copyOf(RoundingMode.values());

  static final ImmutableList<RoundingMode> ALL_SAFE_ROUNDING_MODES =
      ImmutableList.of(DOWN, UP, FLOOR, CEILING, HALF_EVEN, HALF_UP, HALF_DOWN);

    static final ImmutableList<Integer> EXPONENTS =
      ImmutableList.of(0, 1, 2, 3, 4, 7, 10, 15, 20, 25, 40, 70);

  
  private static final Function<Integer, Long> TO_LONG =
      new Function<Integer, Long>() {
        @Override
        public Long apply(Integer n) {
          return Long.valueOf(n);
        }
      };

  
  private static final Function<Long, BigInteger> TO_BIGINTEGER =
      new Function<Long, BigInteger>() {
        @Override
        public BigInteger apply(Long n) {
          return BigInteger.valueOf(n);
        }
      };

  private static final Function<Integer, Integer> NEGATE_INT =
      new Function<Integer, Integer>() {
        @Override
        public Integer apply(Integer x) {
          return -x;
        }
      };

  private static final Function<Long, Long> NEGATE_LONG =
      new Function<Long, Long>() {
        @Override
        public Long apply(Long x) {
          return -x;
        }
      };

  private static final Function<BigInteger, BigInteger> NEGATE_BIGINT =
      new Function<BigInteger, BigInteger>() {
        @Override
        public BigInteger apply(BigInteger x) {
          return x.negate();
        }
      };

  
  static final ImmutableSet<Integer> POSITIVE_INTEGER_CANDIDATES;

  static final Iterable<Integer> NEGATIVE_INTEGER_CANDIDATES;

  static final Iterable<Integer> NONZERO_INTEGER_CANDIDATES;

  static final Iterable<Integer> ALL_INTEGER_CANDIDATES;

  static {
    ImmutableSet.Builder<Integer> intValues = ImmutableSet.builder();
        intValues.add(Integer.MAX_VALUE - 1, Integer.MAX_VALUE);
        for (int i = 1; i <= 40; i++) {
      intValues.add(i);
    }
        for (int exponent : asList(2, 3, 4, 9, 15, 16, 17, 24, 25, 30)) {
      int x = 1 << exponent;
      intValues.add(x, x + 1, x - 1);
    }
    intValues.add(9999).add(10000).add(10001).add(1000000);     intValues.add(5792).add(5793);     POSITIVE_INTEGER_CANDIDATES = intValues.build();
    NEGATIVE_INTEGER_CANDIDATES =
        ImmutableList.copyOf(
            Iterables.concat(
                Iterables.transform(POSITIVE_INTEGER_CANDIDATES, NEGATE_INT),
                ImmutableList.of(Integer.MIN_VALUE)));
    NONZERO_INTEGER_CANDIDATES =
        ImmutableList.copyOf(
            Iterables.concat(POSITIVE_INTEGER_CANDIDATES, NEGATIVE_INTEGER_CANDIDATES));
    ALL_INTEGER_CANDIDATES = Iterables.concat(NONZERO_INTEGER_CANDIDATES, ImmutableList.of(0));
  }

  
  static final ImmutableSet<Long> POSITIVE_LONG_CANDIDATES;

  static final Iterable<Long> NEGATIVE_LONG_CANDIDATES;

  static final Iterable<Long> NONZERO_LONG_CANDIDATES;

  static final Iterable<Long> ALL_LONG_CANDIDATES;

  static {
    ImmutableSet.Builder<Long> longValues = ImmutableSet.builder();
        longValues.addAll(Iterables.transform(POSITIVE_INTEGER_CANDIDATES, TO_LONG));
        longValues.add(Integer.MAX_VALUE + 1L, Long.MAX_VALUE - 1L, Long.MAX_VALUE);

        for (int exponent : asList(32, 33, 39, 40, 41, 47, 48, 49, 55, 56, 57)) {
      long x = 1L << exponent;
      longValues.add(x, x + 1, x - 1);
    }
    longValues.add(194368031998L).add(194368031999L);     POSITIVE_LONG_CANDIDATES = longValues.build();
    NEGATIVE_LONG_CANDIDATES =
        Iterables.concat(
            Iterables.transform(POSITIVE_LONG_CANDIDATES, NEGATE_LONG),
            ImmutableList.of(Long.MIN_VALUE));
    NONZERO_LONG_CANDIDATES = Iterables.concat(POSITIVE_LONG_CANDIDATES, NEGATIVE_LONG_CANDIDATES);
    ALL_LONG_CANDIDATES = Iterables.concat(NONZERO_LONG_CANDIDATES, ImmutableList.of(0L));
  }

  
  static final ImmutableSet<BigInteger> POSITIVE_BIGINTEGER_CANDIDATES;

  static final Iterable<BigInteger> NEGATIVE_BIGINTEGER_CANDIDATES;

  static final Iterable<BigInteger> NONZERO_BIGINTEGER_CANDIDATES;

  static final Iterable<BigInteger> ALL_BIGINTEGER_CANDIDATES;

  static {
    ImmutableSet.Builder<BigInteger> bigValues = ImmutableSet.builder();
        bigValues.addAll(Iterables.transform(POSITIVE_LONG_CANDIDATES, TO_BIGINTEGER));
        bigValues.add(BigInteger.valueOf(Long.MAX_VALUE).add(ONE));
        for (int exponent :
        asList(
            64,
            65,
            71,
            72,
            73,
            79,
            80,
            81,
            255,
            256,
            257,
            511,
            512,
            513,
            Double.MAX_EXPONENT - 1,
            Double.MAX_EXPONENT,
            Double.MAX_EXPONENT + 1)) {
      BigInteger x = ONE.shiftLeft(exponent);
      bigValues.add(x, x.add(ONE), x.subtract(ONE));
    }
    bigValues.add(new BigInteger("218838949120258359057546633"));         bigValues.add(new BigInteger("218838949120258359057546634"));
    POSITIVE_BIGINTEGER_CANDIDATES = bigValues.build();
    NEGATIVE_BIGINTEGER_CANDIDATES =
        Iterables.transform(POSITIVE_BIGINTEGER_CANDIDATES, NEGATE_BIGINT);
    NONZERO_BIGINTEGER_CANDIDATES =
        Iterables.concat(POSITIVE_BIGINTEGER_CANDIDATES, NEGATIVE_BIGINTEGER_CANDIDATES);
    ALL_BIGINTEGER_CANDIDATES =
        Iterables.concat(NONZERO_BIGINTEGER_CANDIDATES, ImmutableList.of(ZERO));
  }

  static final ImmutableSet<Double> INTEGRAL_DOUBLE_CANDIDATES;
  static final ImmutableSet<Double> FRACTIONAL_DOUBLE_CANDIDATES;
  static final Iterable<Double> INFINITIES =
      Doubles.asList(Double.POSITIVE_INFINITY, Double.NEGATIVE_INFINITY);
  static final Iterable<Double> FINITE_DOUBLE_CANDIDATES;
  static final Iterable<Double> POSITIVE_FINITE_DOUBLE_CANDIDATES;
  static final Iterable<Double> ALL_DOUBLE_CANDIDATES;
  static final Iterable<Double> DOUBLE_CANDIDATES_EXCEPT_NAN;

  static {
    ImmutableSet.Builder<Double> integralBuilder = ImmutableSet.builder();
    ImmutableSet.Builder<Double> fractionalBuilder = ImmutableSet.builder();
    integralBuilder.addAll(Doubles.asList(0.0, -0.0, Double.MAX_VALUE, -Double.MAX_VALUE));
        for (int scale = 1; scale <= 4; scale++) {
      for (double d : Doubles.asList(Double.MIN_VALUE, Double.MIN_NORMAL)) {
        fractionalBuilder.add(d * scale).add(-d * scale);
      }
    }
    for (int i = Double.MIN_EXPONENT; i <= Double.MAX_EXPONENT; i++) {
      for (int direction : new int[] {1, -1}) {
        double d = Double.longBitsToDouble(Double.doubleToLongBits(Math.scalb(1.0, i)) + direction);
                if (d != Math.rint(d)) {
          fractionalBuilder.add(d);
        }
      }
    }
    for (double d :
        Doubles.asList(
            0,
            1,
            2,
            7,
            51,
            102,
            Math.scalb(1.0, 53),
            Integer.MIN_VALUE,
            Integer.MAX_VALUE,
            Long.MIN_VALUE,
            Long.MAX_VALUE)) {
      for (double delta : Doubles.asList(0.0, 1.0, 2.0)) {
        integralBuilder.addAll(Doubles.asList(d + delta, d - delta, -d - delta, -d + delta));
      }
      for (double delta : Doubles.asList(0.01, 0.1, 0.25, 0.499, 0.5, 0.501, 0.7, 0.8)) {
        double x = d + delta;
        if (x != Math.round(x)) {
          fractionalBuilder.add(x);
        }
      }
    }
    INTEGRAL_DOUBLE_CANDIDATES = integralBuilder.build();
    fractionalBuilder.add(1.414).add(1.415).add(Math.sqrt(2));
    fractionalBuilder.add(5.656).add(5.657).add(4 * Math.sqrt(2));
    for (double d : INTEGRAL_DOUBLE_CANDIDATES) {
      double x = 1 / d;
      if (x != Math.rint(x)) {
        fractionalBuilder.add(x);
      }
    }
    FRACTIONAL_DOUBLE_CANDIDATES = fractionalBuilder.build();
    FINITE_DOUBLE_CANDIDATES =
        Iterables.concat(FRACTIONAL_DOUBLE_CANDIDATES, INTEGRAL_DOUBLE_CANDIDATES);
    POSITIVE_FINITE_DOUBLE_CANDIDATES =
        Iterables.filter(
            FINITE_DOUBLE_CANDIDATES,
            new Predicate<Double>() {
              @Override
              public boolean apply(Double input) {
                return input.doubleValue() > 0.0;
              }
            });
    DOUBLE_CANDIDATES_EXCEPT_NAN = Iterables.concat(FINITE_DOUBLE_CANDIDATES, INFINITIES);
    ALL_DOUBLE_CANDIDATES = Iterables.concat(DOUBLE_CANDIDATES_EXCEPT_NAN, asList(Double.NaN));
  }
}
