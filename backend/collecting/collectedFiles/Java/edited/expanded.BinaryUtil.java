



package com.google.zxing.oned.rss.expanded;

import com.google.zxing.common.BitArray;

import java.util.regex.Pattern;


public final class BinaryUtil {

  private static final Pattern ONE = Pattern.compile("1");
  private static final Pattern ZERO = Pattern.compile("0");
  private static final Pattern SPACE = Pattern.compile(" ");

  private BinaryUtil() {
  }

  
  public static BitArray buildBitArrayFromString(CharSequence data) {
    CharSequence dotsAndXs = ZERO.matcher(ONE.matcher(data).replaceAll("X")).replaceAll(".");
    BitArray binary = new BitArray(SPACE.matcher(dotsAndXs).replaceAll("").length());
    int counter = 0;

    for (int i = 0; i < dotsAndXs.length(); ++i) {
      if (i % 9 == 0) {         if (dotsAndXs.charAt(i) != ' ') {
          throw new IllegalStateException("space expected");
        }
        continue;
      }

      char currentChar = dotsAndXs.charAt(i);
      if (currentChar == 'X' || currentChar == 'x') {
        binary.set(counter);
      }
      counter++;
    }
    return binary;
  }

  public static BitArray buildBitArrayFromStringWithoutSpaces(CharSequence data) {
    StringBuilder sb = new StringBuilder();
    CharSequence dotsAndXs = ZERO.matcher(ONE.matcher(data).replaceAll("X")).replaceAll(".");
    int current = 0;
    while (current < dotsAndXs.length()) {
      sb.append(' ');
      for (int i = 0; i < 8 && current < dotsAndXs.length(); ++i) {
        sb.append(dotsAndXs.charAt(current));
        current++;
      }
    }
    return buildBitArrayFromString(sb.toString());
  }

}
