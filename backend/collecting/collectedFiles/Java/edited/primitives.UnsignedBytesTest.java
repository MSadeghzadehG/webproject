

package com.google.common.primitives;

import com.google.common.collect.testing.Helpers;
import com.google.common.testing.NullPointerTester;
import com.google.common.testing.SerializableTester;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;
import java.util.Random;
import junit.framework.TestCase;


public class UnsignedBytesTest extends TestCase {
  private static final byte LEAST = 0;
  private static final byte GREATEST = (byte) 255;

    private static final byte[] VALUES = {LEAST, 127, (byte) 128, (byte) 129, GREATEST};

  public void testToInt() {
    assertEquals(0, UnsignedBytes.toInt((byte) 0));
    assertEquals(1, UnsignedBytes.toInt((byte) 1));
    assertEquals(127, UnsignedBytes.toInt((byte) 127));
    assertEquals(128, UnsignedBytes.toInt((byte) -128));
    assertEquals(129, UnsignedBytes.toInt((byte) -127));
    assertEquals(255, UnsignedBytes.toInt((byte) -1));
  }

  public void testCheckedCast() {
    for (byte value : VALUES) {
      assertEquals(value, UnsignedBytes.checkedCast(UnsignedBytes.toInt(value)));
    }
    assertCastFails(256L);
    assertCastFails(-1L);
    assertCastFails(Long.MAX_VALUE);
    assertCastFails(Long.MIN_VALUE);
  }

  public void testSaturatedCast() {
    for (byte value : VALUES) {
      assertEquals(value, UnsignedBytes.saturatedCast(UnsignedBytes.toInt(value)));
    }
    assertEquals(GREATEST, UnsignedBytes.saturatedCast(256L));
    assertEquals(LEAST, UnsignedBytes.saturatedCast(-1L));
    assertEquals(GREATEST, UnsignedBytes.saturatedCast(Long.MAX_VALUE));
    assertEquals(LEAST, UnsignedBytes.saturatedCast(Long.MIN_VALUE));
  }

  private static void assertCastFails(long value) {
    try {
      UnsignedBytes.checkedCast(value);
      fail("Cast to byte should have failed: " + value);
    } catch (IllegalArgumentException ex) {
      assertTrue(
          value + " not found in exception text: " + ex.getMessage(),
          ex.getMessage().contains(String.valueOf(value)));
    }
  }

  public void testCompare() {
            for (int i = 0; i < VALUES.length; i++) {
      for (int j = 0; j < VALUES.length; j++) {
        byte x = VALUES[i];
        byte y = VALUES[j];
                assertEquals(
            x + ", " + y,
            Math.signum(UnsignedBytes.compare(x, y)),
            Math.signum(Ints.compare(i, j)));
      }
    }
  }

  public void testMax_noArgs() {
    try {
      UnsignedBytes.max();
      fail();
    } catch (IllegalArgumentException expected) {
    }
  }

  public void testMax() {
    assertEquals(LEAST, UnsignedBytes.max(LEAST));
    assertEquals(GREATEST, UnsignedBytes.max(GREATEST));
    assertEquals(
        (byte) 255, UnsignedBytes.max((byte) 0, (byte) -128, (byte) -1, (byte) 127, (byte) 1));
  }

  public void testMin_noArgs() {
    try {
      UnsignedBytes.min();
      fail();
    } catch (IllegalArgumentException expected) {
    }
  }

  public void testMin() {
    assertEquals(LEAST, UnsignedBytes.min(LEAST));
    assertEquals(GREATEST, UnsignedBytes.min(GREATEST));
    assertEquals(
        (byte) 0, UnsignedBytes.min((byte) 0, (byte) -128, (byte) -1, (byte) 127, (byte) 1));
    assertEquals(
        (byte) 0, UnsignedBytes.min((byte) -1, (byte) 127, (byte) 1, (byte) -128, (byte) 0));
  }

  private static void assertParseFails(String value) {
    try {
      UnsignedBytes.parseUnsignedByte(value);
      fail();
    } catch (NumberFormatException expected) {
    }
  }

  private static void assertParseFails(String value, int radix) {
    try {
      UnsignedBytes.parseUnsignedByte(value, radix);
      fail();
    } catch (NumberFormatException expected) {
    }
  }

  public void testParseUnsignedByte() {
        for (int i = 0; i <= 0xff; i++) {
      assertEquals((byte) i, UnsignedBytes.parseUnsignedByte(Integer.toString(i)));
    }
    assertParseFails("1000");
    assertParseFails("-1");
    assertParseFails("-128");
    assertParseFails("256");
  }

  public void testMaxValue() {
    assertTrue(
        UnsignedBytes.compare(UnsignedBytes.MAX_VALUE, (byte) (UnsignedBytes.MAX_VALUE + 1)) > 0);
  }

  public void testParseUnsignedByteWithRadix() throws NumberFormatException {
        for (int radix = Character.MIN_RADIX; radix <= Character.MAX_RADIX; radix++) {
      for (int i = 0; i <= 0xff; i++) {
        assertEquals((byte) i, UnsignedBytes.parseUnsignedByte(Integer.toString(i, radix), radix));
      }
      assertParseFails(Integer.toString(1000, radix), radix);
      assertParseFails(Integer.toString(-1, radix), radix);
      assertParseFails(Integer.toString(-128, radix), radix);
      assertParseFails(Integer.toString(256, radix), radix);
    }
  }

  public void testParseUnsignedByteThrowsExceptionForInvalidRadix() {
            try {
      UnsignedBytes.parseUnsignedByte("0", Character.MIN_RADIX - 1);
      fail();
    } catch (NumberFormatException expected) {
    }

    try {
      UnsignedBytes.parseUnsignedByte("0", Character.MAX_RADIX + 1);
      fail();
    } catch (NumberFormatException expected) {
    }

        try {
      UnsignedBytes.parseUnsignedByte("0", -1);
      fail();
    } catch (NumberFormatException expected) {
    }
  }

  public void testToString() {
        for (int i = 0; i <= 0xff; i++) {
      assertEquals(Integer.toString(i), UnsignedBytes.toString((byte) i));
    }
  }

  public void testToStringWithRadix() {
        for (int radix = Character.MIN_RADIX; radix <= Character.MAX_RADIX; radix++) {
      for (int i = 0; i <= 0xff; i++) {
        assertEquals(Integer.toString(i, radix), UnsignedBytes.toString((byte) i, radix));
      }
    }
  }

  public void testJoin() {
    assertEquals("", UnsignedBytes.join(",", new byte[] {}));
    assertEquals("1", UnsignedBytes.join(",", new byte[] {(byte) 1}));
    assertEquals("1,2", UnsignedBytes.join(",", (byte) 1, (byte) 2));
    assertEquals("123", UnsignedBytes.join("", (byte) 1, (byte) 2, (byte) 3));
    assertEquals("128,255", UnsignedBytes.join(",", (byte) 128, (byte) -1));
  }

  private static String unsafeComparatorClassName() {
    return UnsignedBytes.LexicographicalComparatorHolder.class.getName() + "$UnsafeComparator";
  }

  private static boolean unsafeComparatorAvailable() {
            try {
      Class.forName(unsafeComparatorClassName());
      return true;
    } catch (Error | ClassNotFoundException tolerable) {
      
      return false;
    }
  }

  public void testLexicographicalComparatorChoice() throws Exception {
    Comparator<byte[]> defaultComparator = UnsignedBytes.lexicographicalComparator();
    assertNotNull(defaultComparator);
    assertSame(defaultComparator, UnsignedBytes.lexicographicalComparator());
    if (unsafeComparatorAvailable()) {
      assertSame(defaultComparator.getClass(), Class.forName(unsafeComparatorClassName()));
    } else {
      assertSame(defaultComparator, UnsignedBytes.lexicographicalComparatorJavaImpl());
    }
  }

  public void testLexicographicalComparator() {
    List<byte[]> ordered =
        Arrays.asList(
            new byte[] {},
            new byte[] {LEAST},
            new byte[] {LEAST, LEAST},
            new byte[] {LEAST, (byte) 1},
            new byte[] {(byte) 1},
            new byte[] {(byte) 1, LEAST},
            new byte[] {GREATEST, GREATEST - (byte) 1},
            new byte[] {GREATEST, GREATEST},
            new byte[] {GREATEST, GREATEST, GREATEST});

        Comparator<byte[]> comparator = UnsignedBytes.lexicographicalComparator();
    Helpers.testComparator(comparator, ordered);
    assertSame(comparator, SerializableTester.reserialize(comparator));

        Comparator<byte[]> javaImpl = UnsignedBytes.lexicographicalComparatorJavaImpl();
    Helpers.testComparator(javaImpl, ordered);
    assertSame(javaImpl, SerializableTester.reserialize(javaImpl));
  }

  @SuppressWarnings("unchecked")
  public void testLexicographicalComparatorLongInputs() {
    Random rnd = new Random();
    for (Comparator<byte[]> comparator :
        Arrays.asList(
            UnsignedBytes.lexicographicalComparator(),
            UnsignedBytes.lexicographicalComparatorJavaImpl())) {
      for (int trials = 10; trials-- > 0; ) {
        byte[] left = new byte[1 + rnd.nextInt(32)];
        rnd.nextBytes(left);
        byte[] right = left.clone();
        assertTrue(comparator.compare(left, right) == 0);
        int i = rnd.nextInt(left.length);
        left[i] ^= (byte) (1 + rnd.nextInt(255));
        assertTrue(comparator.compare(left, right) != 0);
        assertEquals(
            comparator.compare(left, right) > 0, UnsignedBytes.compare(left[i], right[i]) > 0);
      }
    }
  }

  public void testSort() {
    testSort(new byte[] {}, new byte[] {});
    testSort(new byte[] {2}, new byte[] {2});
    testSort(new byte[] {2, 1, 0}, new byte[] {0, 1, 2});
    testSort(new byte[] {2, GREATEST, 1, LEAST}, new byte[] {LEAST, 1, 2, GREATEST});
  }

  static void testSort(byte[] input, byte[] expected) {
    input = Arrays.copyOf(input, input.length);
    UnsignedBytes.sort(input);
    assertTrue(Arrays.equals(expected, input));
  }

  static void testSort(byte[] input, int from, int to, byte[] expected) {
    input = Arrays.copyOf(input, input.length);
    UnsignedBytes.sort(input, from, to);
    assertTrue(Arrays.equals(expected, input));
  }

  public void testSortIndexed() {
    testSort(new byte[] {}, 0, 0, new byte[] {});
    testSort(new byte[] {2}, 0, 1, new byte[] {2});
    testSort(new byte[] {2, 1, 0}, 0, 2, new byte[] {1, 2, 0});
    testSort(new byte[] {2, GREATEST, 1, LEAST}, 1, 4, new byte[] {2, LEAST, 1, GREATEST});
  }

  public void testSortDescending() {
    testSortDescending(new byte[] {}, new byte[] {});
    testSortDescending(new byte[] {1}, new byte[] {1});
    testSortDescending(new byte[] {1, 2}, new byte[] {2, 1});
    testSortDescending(new byte[] {1, 3, 1}, new byte[] {3, 1, 1});
    testSortDescending(
        new byte[] {GREATEST - 1, 1, GREATEST - 2, 2},
        new byte[] {GREATEST - 1, GREATEST - 2, 2, 1});
  }

  private static void testSortDescending(byte[] input, byte[] expectedOutput) {
    input = Arrays.copyOf(input, input.length);
    UnsignedBytes.sortDescending(input);
    assertTrue(Arrays.equals(expectedOutput, input));
  }

  private static void testSortDescending(
      byte[] input, int fromIndex, int toIndex, byte[] expectedOutput) {
    input = Arrays.copyOf(input, input.length);
    UnsignedBytes.sortDescending(input, fromIndex, toIndex);
    assertTrue(Arrays.equals(expectedOutput, input));
  }

  public void testSortDescendingIndexed() {
    testSortDescending(new byte[] {}, 0, 0, new byte[] {});
    testSortDescending(new byte[] {1}, 0, 1, new byte[] {1});
    testSortDescending(new byte[] {1, 2}, 0, 2, new byte[] {2, 1});
    testSortDescending(new byte[] {1, 3, 1}, 0, 2, new byte[] {3, 1, 1});
    testSortDescending(new byte[] {1, 3, 1}, 0, 1, new byte[] {1, 3, 1});
    testSortDescending(
        new byte[] {GREATEST - 1, 1, GREATEST - 2, 2},
        1,
        3,
        new byte[] {GREATEST - 1, GREATEST - 2, 1, 2});
  }

  public void testNulls() {
    new NullPointerTester().testAllPublicStaticMethods(UnsignedBytes.class);
  }
}
