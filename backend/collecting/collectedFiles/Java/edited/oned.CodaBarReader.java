

package com.google.zxing.oned;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.DecodeHintType;
import com.google.zxing.NotFoundException;
import com.google.zxing.Result;
import com.google.zxing.ResultPoint;
import com.google.zxing.common.BitArray;

import java.util.Arrays;
import java.util.Map;


public final class CodaBarReader extends OneDReader {

        private static final float MAX_ACCEPTABLE = 2.0f;
  private static final float PADDING = 1.5f;

  private static final String ALPHABET_STRING = "0123456789-$:/.+ABCD";
  static final char[] ALPHABET = ALPHABET_STRING.toCharArray();

  
  static final int[] CHARACTER_ENCODINGS = {
      0x003, 0x006, 0x009, 0x060, 0x012, 0x042, 0x021, 0x024, 0x030, 0x048,       0x00c, 0x018, 0x045, 0x051, 0x054, 0x015, 0x01A, 0x029, 0x00B, 0x00E,   };

        private static final int MIN_CHARACTER_LENGTH = 3;

    private static final char[] STARTEND_ENCODING = {'A', 'B', 'C', 'D'};
    
    
    private final StringBuilder decodeRowResult;
  private int[] counters;
  private int counterLength;

  public CodaBarReader() {
    decodeRowResult = new StringBuilder(20);
    counters = new int[80];
    counterLength = 0;
  }

  @Override
  public Result decodeRow(int rowNumber, BitArray row, Map<DecodeHintType,?> hints) throws NotFoundException {

    Arrays.fill(counters, 0);
    setCounters(row);
    int startOffset = findStartPattern();
    int nextStart = startOffset;

    decodeRowResult.setLength(0);
    do {
      int charOffset = toNarrowWidePattern(nextStart);
      if (charOffset == -1) {
        throw NotFoundException.getNotFoundInstance();
      }
                        decodeRowResult.append((char) charOffset);
      nextStart += 8;
            if (decodeRowResult.length() > 1 &&
          arrayContains(STARTEND_ENCODING, ALPHABET[charOffset])) {
        break;
      }
    } while (nextStart < counterLength); 
        int trailingWhitespace = counters[nextStart - 1];
    int lastPatternSize = 0;
    for (int i = -8; i < -1; i++) {
      lastPatternSize += counters[nextStart + i];
    }

                if (nextStart < counterLength && trailingWhitespace < lastPatternSize / 2) {
      throw NotFoundException.getNotFoundInstance();
    }

    validatePattern(startOffset);

        for (int i = 0; i < decodeRowResult.length(); i++) {
      decodeRowResult.setCharAt(i, ALPHABET[decodeRowResult.charAt(i)]);
    }
        char startchar = decodeRowResult.charAt(0);
    if (!arrayContains(STARTEND_ENCODING, startchar)) {
      throw NotFoundException.getNotFoundInstance();
    }
    char endchar = decodeRowResult.charAt(decodeRowResult.length() - 1);
    if (!arrayContains(STARTEND_ENCODING, endchar)) {
      throw NotFoundException.getNotFoundInstance();
    }

        if (decodeRowResult.length() <= MIN_CHARACTER_LENGTH) {
            throw NotFoundException.getNotFoundInstance();
    }

    if (hints == null || !hints.containsKey(DecodeHintType.RETURN_CODABAR_START_END)) {
      decodeRowResult.deleteCharAt(decodeRowResult.length() - 1);
      decodeRowResult.deleteCharAt(0);
    }

    int runningCount = 0;
    for (int i = 0; i < startOffset; i++) {
      runningCount += counters[i];
    }
    float left = runningCount;
    for (int i = startOffset; i < nextStart - 1; i++) {
      runningCount += counters[i];
    }
    float right = runningCount;
    return new Result(
        decodeRowResult.toString(),
        null,
        new ResultPoint[]{
            new ResultPoint(left, rowNumber),
            new ResultPoint(right, rowNumber)},
        BarcodeFormat.CODABAR);
  }

  private void validatePattern(int start) throws NotFoundException {
        int[] sizes = {0, 0, 0, 0};
    int[] counts = {0, 0, 0, 0};
    int end = decodeRowResult.length() - 1;

            int pos = start;
    for (int i = 0; true; i++) {
      int pattern = CHARACTER_ENCODINGS[decodeRowResult.charAt(i)];
      for (int j = 6; j >= 0; j--) {
                        int category = (j & 1) + (pattern & 1) * 2;
        sizes[category] += counters[pos + j];
        counts[category]++;
        pattern >>= 1;
      }
      if (i >= end) {
        break;
      }
            pos += 8;
    }

        float[] maxes = new float[4];
    float[] mins = new float[4];
                for (int i = 0; i < 2; i++) {
      mins[i] = 0.0f;        mins[i + 2] = ((float) sizes[i] / counts[i] + (float) sizes[i + 2] / counts[i + 2]) / 2.0f;
      maxes[i] = mins[i + 2];
      maxes[i + 2] = (sizes[i + 2] * MAX_ACCEPTABLE + PADDING) / counts[i + 2];
    }

        pos = start;
    for (int i = 0; true; i++) {
      int pattern = CHARACTER_ENCODINGS[decodeRowResult.charAt(i)];
      for (int j = 6; j >= 0; j--) {
                        int category = (j & 1) + (pattern & 1) * 2;
        int size = counters[pos + j];
        if (size < mins[category] || size > maxes[category]) {
          throw NotFoundException.getNotFoundInstance();
        }
        pattern >>= 1;
      }
      if (i >= end) {
        break;
      }
      pos += 8;
    }
  }

  
  private void setCounters(BitArray row) throws NotFoundException {
    counterLength = 0;
        int i = row.getNextUnset(0);
    int end = row.getSize();
    if (i >= end) {
      throw NotFoundException.getNotFoundInstance();
    }
    boolean isWhite = true;
    int count = 0;
    while (i < end) {
      if (row.get(i) != isWhite) {
        count++;
      } else {
        counterAppend(count);
        count = 1;
        isWhite = !isWhite;
      }
      i++;
    }
    counterAppend(count);
  }

  private void counterAppend(int e) {
    counters[counterLength] = e;
    counterLength++;
    if (counterLength >= counters.length) {
      int[] temp = new int[counterLength * 2];
      System.arraycopy(counters, 0, temp, 0, counterLength);
      counters = temp;
    }
  }

  private int findStartPattern() throws NotFoundException {
    for (int i = 1; i < counterLength; i += 2) {
      int charOffset = toNarrowWidePattern(i);
      if (charOffset != -1 && arrayContains(STARTEND_ENCODING, ALPHABET[charOffset])) {
                        int patternSize = 0;
        for (int j = i; j < i + 7; j++) {
          patternSize += counters[j];
        }
        if (i == 1 || counters[i - 1] >= patternSize / 2) {
          return i;
        }
      }
    }
    throw NotFoundException.getNotFoundInstance();
  }

  static boolean arrayContains(char[] array, char key) {
    if (array != null) {
      for (char c : array) {
        if (c == key) {
          return true;
        }
      }
    }
    return false;
  }

    private int toNarrowWidePattern(int position) {
    int end = position + 7;
    if (end >= counterLength) {
      return -1;
    }

    int[] theCounters = counters;

    int maxBar = 0;
    int minBar = Integer.MAX_VALUE;
    for (int j = position; j < end; j += 2) {
      int currentCounter = theCounters[j];
      if (currentCounter < minBar) {
        minBar = currentCounter;
      }
      if (currentCounter > maxBar) {
        maxBar = currentCounter;
      }
    }
    int thresholdBar = (minBar + maxBar) / 2;

    int maxSpace = 0;
    int minSpace = Integer.MAX_VALUE;
    for (int j = position + 1; j < end; j += 2) {
      int currentCounter = theCounters[j];
      if (currentCounter < minSpace) {
        minSpace = currentCounter;
      }
      if (currentCounter > maxSpace) {
        maxSpace = currentCounter;
      }
    }
    int thresholdSpace = (minSpace + maxSpace) / 2;

    int bitmask = 1 << 7;
    int pattern = 0;
    for (int i = 0; i < 7; i++) {
      int threshold = (i & 1) == 0 ? thresholdBar : thresholdSpace;
      bitmask >>= 1;
      if (theCounters[position + i] > threshold) {
        pattern |= bitmask;
      }
    }

    for (int i = 0; i < CHARACTER_ENCODINGS.length; i++) {
      if (CHARACTER_ENCODINGS[i] == pattern) {
        return i;
      }
    }
    return -1;
  }

}
