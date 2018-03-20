

package com.google.zxing.oned;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.ChecksumException;
import com.google.zxing.DecodeHintType;
import com.google.zxing.FormatException;
import com.google.zxing.NotFoundException;
import com.google.zxing.Result;
import com.google.zxing.ResultPoint;
import com.google.zxing.common.BitArray;

import java.util.Arrays;
import java.util.Map;


public final class Code39Reader extends OneDReader {

  static final String ALPHABET_STRING = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";

  
  static final int[] CHARACTER_ENCODINGS = {
      0x034, 0x121, 0x061, 0x160, 0x031, 0x130, 0x070, 0x025, 0x124, 0x064,       0x109, 0x049, 0x148, 0x019, 0x118, 0x058, 0x00D, 0x10C, 0x04C, 0x01C,       0x103, 0x043, 0x142, 0x013, 0x112, 0x052, 0x007, 0x106, 0x046, 0x016,       0x181, 0x0C1, 0x1C0, 0x091, 0x190, 0x0D0, 0x085, 0x184, 0x0C4, 0x0A8,       0x0A2, 0x08A, 0x02A   };

  static final int ASTERISK_ENCODING = 0x094;

  private final boolean usingCheckDigit;
  private final boolean extendedMode;
  private final StringBuilder decodeRowResult;
  private final int[] counters;

  
  public Code39Reader() {
    this(false);
  }

  
  public Code39Reader(boolean usingCheckDigit) {
    this(usingCheckDigit, false);
  }

  
  public Code39Reader(boolean usingCheckDigit, boolean extendedMode) {
    this.usingCheckDigit = usingCheckDigit;
    this.extendedMode = extendedMode;
    decodeRowResult = new StringBuilder(20);
    counters = new int[9];
  }

  @Override
  public Result decodeRow(int rowNumber, BitArray row, Map<DecodeHintType,?> hints)
      throws NotFoundException, ChecksumException, FormatException {

    int[] theCounters = counters;
    Arrays.fill(theCounters, 0);
    StringBuilder result = decodeRowResult;
    result.setLength(0);

    int[] start = findAsteriskPattern(row, theCounters);
        int nextStart = row.getNextSet(start[1]);
    int end = row.getSize();

    char decodedChar;
    int lastStart;
    do {
      recordPattern(row, nextStart, theCounters);
      int pattern = toNarrowWidePattern(theCounters);
      if (pattern < 0) {
        throw NotFoundException.getNotFoundInstance();
      }
      decodedChar = patternToChar(pattern);
      result.append(decodedChar);
      lastStart = nextStart;
      for (int counter : theCounters) {
        nextStart += counter;
      }
            nextStart = row.getNextSet(nextStart);
    } while (decodedChar != '*');
    result.setLength(result.length() - 1); 
        int lastPatternSize = 0;
    for (int counter : theCounters) {
      lastPatternSize += counter;
    }
    int whiteSpaceAfterEnd = nextStart - lastStart - lastPatternSize;
            if (nextStart != end && (whiteSpaceAfterEnd * 2) < lastPatternSize) {
      throw NotFoundException.getNotFoundInstance();
    }

    if (usingCheckDigit) {
      int max = result.length() - 1;
      int total = 0;
      for (int i = 0; i < max; i++) {
        total += ALPHABET_STRING.indexOf(decodeRowResult.charAt(i));
      }
      if (result.charAt(max) != ALPHABET_STRING.charAt(total % 43)) {
        throw ChecksumException.getChecksumInstance();
      }
      result.setLength(max);
    }

    if (result.length() == 0) {
            throw NotFoundException.getNotFoundInstance();
    }

    String resultString;
    if (extendedMode) {
      resultString = decodeExtended(result);
    } else {
      resultString = result.toString();
    }

    float left = (start[1] + start[0]) / 2.0f;
    float right = lastStart + lastPatternSize / 2.0f;
    return new Result(
        resultString,
        null,
        new ResultPoint[]{
            new ResultPoint(left, rowNumber),
            new ResultPoint(right, rowNumber)},
        BarcodeFormat.CODE_39);

  }

  private static int[] findAsteriskPattern(BitArray row, int[] counters) throws NotFoundException {
    int width = row.getSize();
    int rowOffset = row.getNextSet(0);

    int counterPosition = 0;
    int patternStart = rowOffset;
    boolean isWhite = false;
    int patternLength = counters.length;

    for (int i = rowOffset; i < width; i++) {
      if (row.get(i) != isWhite) {
        counters[counterPosition]++;
      } else {
        if (counterPosition == patternLength - 1) {
                    if (toNarrowWidePattern(counters) == ASTERISK_ENCODING &&
              row.isRange(Math.max(0, patternStart - ((i - patternStart) / 2)), patternStart, false)) {
            return new int[]{patternStart, i};
          }
          patternStart += counters[0] + counters[1];
          System.arraycopy(counters, 2, counters, 0, counterPosition - 1);
          counters[counterPosition - 1] = 0;
          counters[counterPosition] = 0;
          counterPosition--;
        } else {
          counterPosition++;
        }
        counters[counterPosition] = 1;
        isWhite = !isWhite;
      }
    }
    throw NotFoundException.getNotFoundInstance();
  }

      private static int toNarrowWidePattern(int[] counters) {
    int numCounters = counters.length;
    int maxNarrowCounter = 0;
    int wideCounters;
    do {
      int minCounter = Integer.MAX_VALUE;
      for (int counter : counters) {
        if (counter < minCounter && counter > maxNarrowCounter) {
          minCounter = counter;
        }
      }
      maxNarrowCounter = minCounter;
      wideCounters = 0;
      int totalWideCountersWidth = 0;
      int pattern = 0;
      for (int i = 0; i < numCounters; i++) {
        int counter = counters[i];
        if (counter > maxNarrowCounter) {
          pattern |= 1 << (numCounters - 1 - i);
          wideCounters++;
          totalWideCountersWidth += counter;
        }
      }
      if (wideCounters == 3) {
                                for (int i = 0; i < numCounters && wideCounters > 0; i++) {
          int counter = counters[i];
          if (counter > maxNarrowCounter) {
            wideCounters--;
                        if ((counter * 2) >= totalWideCountersWidth) {
              return -1;
            }
          }
        }
        return pattern;
      }
    } while (wideCounters > 3);
    return -1;
  }

  private static char patternToChar(int pattern) throws NotFoundException {
    for (int i = 0; i < CHARACTER_ENCODINGS.length; i++) {
      if (CHARACTER_ENCODINGS[i] == pattern) {
        return ALPHABET_STRING.charAt(i);
      }
    }
    if (pattern == ASTERISK_ENCODING) {
      return '*';
    }
    throw NotFoundException.getNotFoundInstance();
  }

  private static String decodeExtended(CharSequence encoded) throws FormatException {
    int length = encoded.length();
    StringBuilder decoded = new StringBuilder(length);
    for (int i = 0; i < length; i++) {
      char c = encoded.charAt(i);
      if (c == '+' || c == '$' || c == '%' || c == '/') {
        char next = encoded.charAt(i + 1);
        char decodedChar = '\0';
        switch (c) {
          case '+':
                        if (next >= 'A' && next <= 'Z') {
              decodedChar = (char) (next + 32);
            } else {
              throw FormatException.getFormatInstance();
            }
            break;
          case '$':
                        if (next >= 'A' && next <= 'Z') {
              decodedChar = (char) (next - 64);
            } else {
              throw FormatException.getFormatInstance();
            }
            break;
          case '%':
                        if (next >= 'A' && next <= 'E') {
              decodedChar = (char) (next - 38);
            } else if (next >= 'F' && next <= 'J') {
              decodedChar = (char) (next - 11);
            } else if (next >= 'K' && next <= 'O') {
              decodedChar = (char) (next + 16);
            } else if (next >= 'P' && next <= 'T') {
              decodedChar = (char) (next + 43);
            } else if (next == 'U') {
              decodedChar = (char) 0;
            } else if (next == 'V') {
              decodedChar = '@';
            } else if (next == 'W') {
              decodedChar = '`';
            } else if (next == 'X' || next == 'Y' || next == 'Z') {
              decodedChar = (char) 127;
            } else {
              throw FormatException.getFormatInstance();
            }
            break;
          case '/':
                        if (next >= 'A' && next <= 'O') {
              decodedChar = (char) (next - 32);
            } else if (next == 'Z') {
              decodedChar = ':';
            } else {
              throw FormatException.getFormatInstance();
            }
            break;
        }
        decoded.append(decodedChar);
                i++;
      } else {
        decoded.append(c);
      }
    }
    return decoded.toString();
  }

}
