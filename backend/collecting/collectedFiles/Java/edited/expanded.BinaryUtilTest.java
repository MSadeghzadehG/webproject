



package com.google.zxing.oned.rss.expanded;

import com.google.zxing.common.BitArray;

import org.junit.Assert;
import org.junit.Test;

import java.util.regex.Pattern;


public final class BinaryUtilTest extends Assert {

   private static final Pattern SPACE = Pattern.compile(" ");

  @Test
  public void testBuildBitArrayFromString() {

    CharSequence data = " ..X..X.. ..XXX... XXXXXXXX ........";
    check(data);

    data = " XXX..X..";
    check(data);

    data = " XX";
    check(data);

    data = " ....XX.. ..XX";
    check(data);

    data = " ....XX.. ..XX..XX ....X.X. ........";
    check(data);
  }

  private static void check(CharSequence data) {
    BitArray binary = BinaryUtil.buildBitArrayFromString(data);
    assertEquals(data, binary.toString());
  }

  @Test
  public void testBuildBitArrayFromStringWithoutSpaces() {
    CharSequence data = " ..X..X.. ..XXX... XXXXXXXX ........";
    checkWithoutSpaces(data);

    data = " XXX..X..";
    checkWithoutSpaces(data);

    data = " XX";
    checkWithoutSpaces(data);

    data = " ....XX.. ..XX";
    checkWithoutSpaces(data);

    data = " ....XX.. ..XX..XX ....X.X. ........";
    checkWithoutSpaces(data);
  }

  private static void checkWithoutSpaces(CharSequence data) {
    CharSequence dataWithoutSpaces = SPACE.matcher(data).replaceAll("");
    BitArray binary = BinaryUtil.buildBitArrayFromStringWithoutSpaces(dataWithoutSpaces);
    assertEquals(data, binary.toString());
  }
}
