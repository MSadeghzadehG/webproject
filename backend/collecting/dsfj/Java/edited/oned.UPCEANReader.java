

package com.google.zxing.oned;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.ChecksumException;
import com.google.zxing.DecodeHintType;
import com.google.zxing.FormatException;
import com.google.zxing.NotFoundException;
import com.google.zxing.ReaderException;
import com.google.zxing.Result;
import com.google.zxing.ResultMetadataType;
import com.google.zxing.ResultPoint;
import com.google.zxing.ResultPointCallback;
import com.google.zxing.common.BitArray;

import java.util.Arrays;
import java.util.Map;


public abstract class UPCEANReader extends OneDReader {

        private static final float MAX_AVG_VARIANCE = 0.48f;
  private static final float MAX_INDIVIDUAL_VARIANCE = 0.7f;

  
  static final int[] START_END_PATTERN = {1, 1, 1,};

  
  static final int[] MIDDLE_PATTERN = {1, 1, 1, 1, 1};
  
  static final int[] END_PATTERN = {1, 1, 1, 1, 1, 1};
  
  static final int[][] L_PATTERNS = {
      {3, 2, 1, 1},       {2, 2, 2, 1},       {2, 1, 2, 2},       {1, 4, 1, 1},       {1, 1, 3, 2},       {1, 2, 3, 1},       {1, 1, 1, 4},       {1, 3, 1, 2},       {1, 2, 1, 3},       {3, 1, 1, 2}    };

  
  static final int[][] L_AND_G_PATTERNS;

  static {
    L_AND_G_PATTERNS = new int[20][];
    System.arraycopy(L_PATTERNS, 0, L_AND_G_PATTERNS, 0, 10);
    for (int i = 10; i < 20; i++) {
      int[] widths = L_PATTERNS[i - 10];
      int[] reversedWidths = new int[widths.length];
      for (int j = 0; j < widths.length; j++) {
        reversedWidths[j] = widths[widths.length - j - 1];
      }
      L_AND_G_PATTERNS[i] = reversedWidths;
    }
  }

  private final StringBuilder decodeRowStringBuffer;
  private final UPCEANExtensionSupport extensionReader;
  private final EANManufacturerOrgSupport eanManSupport;

  protected UPCEANReader() {
    decodeRowStringBuffer = new StringBuilder(20);
    extensionReader = new UPCEANExtensionSupport();
    eanManSupport = new EANManufacturerOrgSupport();
  }

  static int[] findStartGuardPattern(BitArray row) throws NotFoundException {
    boolean foundStart = false;
    int[] startRange = null;
    int nextStart = 0;
    int[] counters = new int[START_END_PATTERN.length];
    while (!foundStart) {
      Arrays.fill(counters, 0, START_END_PATTERN.length, 0);
      startRange = findGuardPattern(row, nextStart, false, START_END_PATTERN, counters);
      int start = startRange[0];
      nextStart = startRange[1];
                        int quietStart = start - (nextStart - start);
      if (quietStart >= 0) {
        foundStart = row.isRange(quietStart, start, false);
      }
    }
    return startRange;
  }

  @Override
  public Result decodeRow(int rowNumber, BitArray row, Map<DecodeHintType,?> hints)
      throws NotFoundException, ChecksumException, FormatException {
    return decodeRow(rowNumber, row, findStartGuardPattern(row), hints);
  }

  
  public Result decodeRow(int rowNumber,
                          BitArray row,
                          int[] startGuardRange,
                          Map<DecodeHintType,?> hints)
      throws NotFoundException, ChecksumException, FormatException {

    ResultPointCallback resultPointCallback = hints == null ? null :
        (ResultPointCallback) hints.get(DecodeHintType.NEED_RESULT_POINT_CALLBACK);

    if (resultPointCallback != null) {
      resultPointCallback.foundPossibleResultPoint(new ResultPoint(
          (startGuardRange[0] + startGuardRange[1]) / 2.0f, rowNumber
      ));
    }

    StringBuilder result = decodeRowStringBuffer;
    result.setLength(0);
    int endStart = decodeMiddle(row, startGuardRange, result);

    if (resultPointCallback != null) {
      resultPointCallback.foundPossibleResultPoint(new ResultPoint(
          endStart, rowNumber
      ));
    }

    int[] endRange = decodeEnd(row, endStart);

    if (resultPointCallback != null) {
      resultPointCallback.foundPossibleResultPoint(new ResultPoint(
          (endRange[0] + endRange[1]) / 2.0f, rowNumber
      ));
    }


            int end = endRange[1];
    int quietEnd = end + (end - endRange[0]);
    if (quietEnd >= row.getSize() || !row.isRange(end, quietEnd, false)) {
      throw NotFoundException.getNotFoundInstance();
    }

    String resultString = result.toString();
        if (resultString.length() < 8) {
      throw FormatException.getFormatInstance();
    }
    if (!checkChecksum(resultString)) {
      throw ChecksumException.getChecksumInstance();
    }

    float left = (startGuardRange[1] + startGuardRange[0]) / 2.0f;
    float right = (endRange[1] + endRange[0]) / 2.0f;
    BarcodeFormat format = getBarcodeFormat();
    Result decodeResult = new Result(resultString,
        null,         new ResultPoint[]{
            new ResultPoint(left, rowNumber),
            new ResultPoint(right, rowNumber)},
        format);

    int extensionLength = 0;

    try {
      Result extensionResult = extensionReader.decodeRow(rowNumber, row, endRange[1]);
      decodeResult.putMetadata(ResultMetadataType.UPC_EAN_EXTENSION, extensionResult.getText());
      decodeResult.putAllMetadata(extensionResult.getResultMetadata());
      decodeResult.addResultPoints(extensionResult.getResultPoints());
      extensionLength = extensionResult.getText().length();
    } catch (ReaderException re) {
          }

    int[] allowedExtensions =
        hints == null ? null : (int[]) hints.get(DecodeHintType.ALLOWED_EAN_EXTENSIONS);
    if (allowedExtensions != null) {
      boolean valid = false;
      for (int length : allowedExtensions) {
        if (extensionLength == length) {
          valid = true;
          break;
        }
      }
      if (!valid) {
        throw NotFoundException.getNotFoundInstance();
      }
    }

    if (format == BarcodeFormat.EAN_13 || format == BarcodeFormat.UPC_A) {
      String countryID = eanManSupport.lookupCountryIdentifier(resultString);
      if (countryID != null) {
        decodeResult.putMetadata(ResultMetadataType.POSSIBLE_COUNTRY, countryID);
      }
    }

    return decodeResult;
  }

  
  boolean checkChecksum(String s) throws FormatException {
    return checkStandardUPCEANChecksum(s);
  }

  
  static boolean checkStandardUPCEANChecksum(CharSequence s) throws FormatException {
    int length = s.length();
    if (length == 0) {
      return false;
    }
    int check = Character.digit(s.charAt(length - 1), 10);
    return getStandardUPCEANChecksum(s.subSequence(0, length - 1)) == check;
  }

  static int getStandardUPCEANChecksum(CharSequence s) throws FormatException {
    int length = s.length();
    int sum = 0;
    for (int i = length - 1; i >= 0; i -= 2) {
      int digit = s.charAt(i) - '0';
      if (digit < 0 || digit > 9) {
        throw FormatException.getFormatInstance();
      }
      sum += digit;
    }
    sum *= 3;
    for (int i = length - 2; i >= 0; i -= 2) {
      int digit = s.charAt(i) - '0';
      if (digit < 0 || digit > 9) {
        throw FormatException.getFormatInstance();
      }
      sum += digit;
    }
    return (1000 - sum) % 10;
  }

  int[] decodeEnd(BitArray row, int endStart) throws NotFoundException {
    return findGuardPattern(row, endStart, false, START_END_PATTERN);
  }

  static int[] findGuardPattern(BitArray row,
                                int rowOffset,
                                boolean whiteFirst,
                                int[] pattern) throws NotFoundException {
    return findGuardPattern(row, rowOffset, whiteFirst, pattern, new int[pattern.length]);
  }

  
  private static int[] findGuardPattern(BitArray row,
                                        int rowOffset,
                                        boolean whiteFirst,
                                        int[] pattern,
                                        int[] counters) throws NotFoundException {
    int width = row.getSize();
    rowOffset = whiteFirst ? row.getNextUnset(rowOffset) : row.getNextSet(rowOffset);
    int counterPosition = 0;
    int patternStart = rowOffset;
    int patternLength = pattern.length;
    boolean isWhite = whiteFirst;
    for (int x = rowOffset; x < width; x++) {
      if (row.get(x) != isWhite) {
        counters[counterPosition]++;
      } else {
        if (counterPosition == patternLength - 1) {
          if (patternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE) {
            return new int[]{patternStart, x};
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

  
  static int decodeDigit(BitArray row, int[] counters, int rowOffset, int[][] patterns)
      throws NotFoundException {
    recordPattern(row, rowOffset, counters);
    float bestVariance = MAX_AVG_VARIANCE;     int bestMatch = -1;
    int max = patterns.length;
    for (int i = 0; i < max; i++) {
      int[] pattern = patterns[i];
      float variance = patternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE);
      if (variance < bestVariance) {
        bestVariance = variance;
        bestMatch = i;
      }
    }
    if (bestMatch >= 0) {
      return bestMatch;
    } else {
      throw NotFoundException.getNotFoundInstance();
    }
  }

  
  abstract BarcodeFormat getBarcodeFormat();

  
  protected abstract int decodeMiddle(BitArray row,
                                      int[] startRange,
                                      StringBuilder resultString) throws NotFoundException;

}
