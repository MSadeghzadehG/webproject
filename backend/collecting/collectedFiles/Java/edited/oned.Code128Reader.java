

package com.google.zxing.oned;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.ChecksumException;
import com.google.zxing.DecodeHintType;
import com.google.zxing.FormatException;
import com.google.zxing.NotFoundException;
import com.google.zxing.Result;
import com.google.zxing.ResultPoint;
import com.google.zxing.common.BitArray;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;


public final class Code128Reader extends OneDReader {

  static final int[][] CODE_PATTERNS = {
      {2, 1, 2, 2, 2, 2},       {2, 2, 2, 1, 2, 2},
      {2, 2, 2, 2, 2, 1},
      {1, 2, 1, 2, 2, 3},
      {1, 2, 1, 3, 2, 2},
      {1, 3, 1, 2, 2, 2},       {1, 2, 2, 2, 1, 3},
      {1, 2, 2, 3, 1, 2},
      {1, 3, 2, 2, 1, 2},
      {2, 2, 1, 2, 1, 3},
      {2, 2, 1, 3, 1, 2},       {2, 3, 1, 2, 1, 2},
      {1, 1, 2, 2, 3, 2},
      {1, 2, 2, 1, 3, 2},
      {1, 2, 2, 2, 3, 1},
      {1, 1, 3, 2, 2, 2},       {1, 2, 3, 1, 2, 2},
      {1, 2, 3, 2, 2, 1},
      {2, 2, 3, 2, 1, 1},
      {2, 2, 1, 1, 3, 2},
      {2, 2, 1, 2, 3, 1},       {2, 1, 3, 2, 1, 2},
      {2, 2, 3, 1, 1, 2},
      {3, 1, 2, 1, 3, 1},
      {3, 1, 1, 2, 2, 2},
      {3, 2, 1, 1, 2, 2},       {3, 2, 1, 2, 2, 1},
      {3, 1, 2, 2, 1, 2},
      {3, 2, 2, 1, 1, 2},
      {3, 2, 2, 2, 1, 1},
      {2, 1, 2, 1, 2, 3},       {2, 1, 2, 3, 2, 1},
      {2, 3, 2, 1, 2, 1},
      {1, 1, 1, 3, 2, 3},
      {1, 3, 1, 1, 2, 3},
      {1, 3, 1, 3, 2, 1},       {1, 1, 2, 3, 1, 3},
      {1, 3, 2, 1, 1, 3},
      {1, 3, 2, 3, 1, 1},
      {2, 1, 1, 3, 1, 3},
      {2, 3, 1, 1, 1, 3},       {2, 3, 1, 3, 1, 1},
      {1, 1, 2, 1, 3, 3},
      {1, 1, 2, 3, 3, 1},
      {1, 3, 2, 1, 3, 1},
      {1, 1, 3, 1, 2, 3},       {1, 1, 3, 3, 2, 1},
      {1, 3, 3, 1, 2, 1},
      {3, 1, 3, 1, 2, 1},
      {2, 1, 1, 3, 3, 1},
      {2, 3, 1, 1, 3, 1},       {2, 1, 3, 1, 1, 3},
      {2, 1, 3, 3, 1, 1},
      {2, 1, 3, 1, 3, 1},
      {3, 1, 1, 1, 2, 3},
      {3, 1, 1, 3, 2, 1},       {3, 3, 1, 1, 2, 1},
      {3, 1, 2, 1, 1, 3},
      {3, 1, 2, 3, 1, 1},
      {3, 3, 2, 1, 1, 1},
      {3, 1, 4, 1, 1, 1},       {2, 2, 1, 4, 1, 1},
      {4, 3, 1, 1, 1, 1},
      {1, 1, 1, 2, 2, 4},
      {1, 1, 1, 4, 2, 2},
      {1, 2, 1, 1, 2, 4},       {1, 2, 1, 4, 2, 1},
      {1, 4, 1, 1, 2, 2},
      {1, 4, 1, 2, 2, 1},
      {1, 1, 2, 2, 1, 4},
      {1, 1, 2, 4, 1, 2},       {1, 2, 2, 1, 1, 4},
      {1, 2, 2, 4, 1, 1},
      {1, 4, 2, 1, 1, 2},
      {1, 4, 2, 2, 1, 1},
      {2, 4, 1, 2, 1, 1},       {2, 2, 1, 1, 1, 4},
      {4, 1, 3, 1, 1, 1},
      {2, 4, 1, 1, 1, 2},
      {1, 3, 4, 1, 1, 1},
      {1, 1, 1, 2, 4, 2},       {1, 2, 1, 1, 4, 2},
      {1, 2, 1, 2, 4, 1},
      {1, 1, 4, 2, 1, 2},
      {1, 2, 4, 1, 1, 2},
      {1, 2, 4, 2, 1, 1},       {4, 1, 1, 2, 1, 2},
      {4, 2, 1, 1, 1, 2},
      {4, 2, 1, 2, 1, 1},
      {2, 1, 2, 1, 4, 1},
      {2, 1, 4, 1, 2, 1},       {4, 1, 2, 1, 2, 1},
      {1, 1, 1, 1, 4, 3},
      {1, 1, 1, 3, 4, 1},
      {1, 3, 1, 1, 4, 1},
      {1, 1, 4, 1, 1, 3},       {1, 1, 4, 3, 1, 1},
      {4, 1, 1, 1, 1, 3},
      {4, 1, 1, 3, 1, 1},
      {1, 1, 3, 1, 4, 1},
      {1, 1, 4, 1, 3, 1},       {3, 1, 1, 1, 4, 1},
      {4, 1, 1, 1, 3, 1},
      {2, 1, 1, 4, 1, 2},
      {2, 1, 1, 2, 1, 4},
      {2, 1, 1, 2, 3, 2},       {2, 3, 3, 1, 1, 1, 2}
  };

  private static final float MAX_AVG_VARIANCE = 0.25f;
  private static final float MAX_INDIVIDUAL_VARIANCE = 0.7f;

  private static final int CODE_SHIFT = 98;

  private static final int CODE_CODE_C = 99;
  private static final int CODE_CODE_B = 100;
  private static final int CODE_CODE_A = 101;

  private static final int CODE_FNC_1 = 102;
  private static final int CODE_FNC_2 = 97;
  private static final int CODE_FNC_3 = 96;
  private static final int CODE_FNC_4_A = 101;
  private static final int CODE_FNC_4_B = 100;

  private static final int CODE_START_A = 103;
  private static final int CODE_START_B = 104;
  private static final int CODE_START_C = 105;
  private static final int CODE_STOP = 106;

  private static int[] findStartPattern(BitArray row) throws NotFoundException {
    int width = row.getSize();
    int rowOffset = row.getNextSet(0);

    int counterPosition = 0;
    int[] counters = new int[6];
    int patternStart = rowOffset;
    boolean isWhite = false;
    int patternLength = counters.length;

    for (int i = rowOffset; i < width; i++) {
      if (row.get(i) != isWhite) {
        counters[counterPosition]++;
      } else {
        if (counterPosition == patternLength - 1) {
          float bestVariance = MAX_AVG_VARIANCE;
          int bestMatch = -1;
          for (int startCode = CODE_START_A; startCode <= CODE_START_C; startCode++) {
            float variance = patternMatchVariance(counters, CODE_PATTERNS[startCode],
                MAX_INDIVIDUAL_VARIANCE);
            if (variance < bestVariance) {
              bestVariance = variance;
              bestMatch = startCode;
            }
          }
                    if (bestMatch >= 0 &&
              row.isRange(Math.max(0, patternStart - (i - patternStart) / 2), patternStart, false)) {
            return new int[]{patternStart, i, bestMatch};
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

  private static int decodeCode(BitArray row, int[] counters, int rowOffset)
      throws NotFoundException {
    recordPattern(row, rowOffset, counters);
    float bestVariance = MAX_AVG_VARIANCE;     int bestMatch = -1;
    for (int d = 0; d < CODE_PATTERNS.length; d++) {
      int[] pattern = CODE_PATTERNS[d];
      float variance = patternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE);
      if (variance < bestVariance) {
        bestVariance = variance;
        bestMatch = d;
      }
    }
        if (bestMatch >= 0) {
      return bestMatch;
    } else {
      throw NotFoundException.getNotFoundInstance();
    }
  }

  @Override
  public Result decodeRow(int rowNumber, BitArray row, Map<DecodeHintType,?> hints)
      throws NotFoundException, FormatException, ChecksumException {

    boolean convertFNC1 = hints != null && hints.containsKey(DecodeHintType.ASSUME_GS1);

    int[] startPatternInfo = findStartPattern(row);
    int startCode = startPatternInfo[2];

    List<Byte> rawCodes = new ArrayList<>(20);
    rawCodes.add((byte) startCode);

    int codeSet;
    switch (startCode) {
      case CODE_START_A:
        codeSet = CODE_CODE_A;
        break;
      case CODE_START_B:
        codeSet = CODE_CODE_B;
        break;
      case CODE_START_C:
        codeSet = CODE_CODE_C;
        break;
      default:
        throw FormatException.getFormatInstance();
    }

    boolean done = false;
    boolean isNextShifted = false;

    StringBuilder result = new StringBuilder(20);

    int lastStart = startPatternInfo[0];
    int nextStart = startPatternInfo[1];
    int[] counters = new int[6];

    int lastCode = 0;
    int code = 0;
    int checksumTotal = startCode;
    int multiplier = 0;
    boolean lastCharacterWasPrintable = true;
    boolean upperMode = false;
    boolean shiftUpperMode = false;

    while (!done) {

      boolean unshift = isNextShifted;
      isNextShifted = false;

            lastCode = code;

            code = decodeCode(row, counters, nextStart);

      rawCodes.add((byte) code);

            if (code != CODE_STOP) {
        lastCharacterWasPrintable = true;
      }

            if (code != CODE_STOP) {
        multiplier++;
        checksumTotal += multiplier * code;
      }

            lastStart = nextStart;
      for (int counter : counters) {
        nextStart += counter;
      }

            switch (code) {
        case CODE_START_A:
        case CODE_START_B:
        case CODE_START_C:
          throw FormatException.getFormatInstance();
      }

      switch (codeSet) {

        case CODE_CODE_A:
          if (code < 64) {
            if (shiftUpperMode == upperMode) {
              result.append((char) (' ' + code));
            } else {
              result.append((char) (' ' + code + 128));
            }
            shiftUpperMode = false;
          } else if (code < 96) {
            if (shiftUpperMode == upperMode) {
              result.append((char) (code - 64));
            } else {
              result.append((char) (code + 64));
            }
            shiftUpperMode = false;
          } else {
                                    if (code != CODE_STOP) {
              lastCharacterWasPrintable = false;
            }
            switch (code) {
              case CODE_FNC_1:
                if (convertFNC1) {
                  if (result.length() == 0) {
                                                            result.append("]C1");
                  } else {
                                        result.append((char) 29);
                  }
                }
                break;
              case CODE_FNC_2:
              case CODE_FNC_3:
                                break;
              case CODE_FNC_4_A:
                if (!upperMode && shiftUpperMode) {
                  upperMode = true;
                  shiftUpperMode = false;
                } else if (upperMode && shiftUpperMode) {
                  upperMode = false;
                  shiftUpperMode = false;
                } else {
                  shiftUpperMode = true;
                }
                break;
              case CODE_SHIFT:
                isNextShifted = true;
                codeSet = CODE_CODE_B;
                break;
              case CODE_CODE_B:
                codeSet = CODE_CODE_B;
                break;
              case CODE_CODE_C:
                codeSet = CODE_CODE_C;
                break;
              case CODE_STOP:
                done = true;
                break;
            }
          }
          break;
        case CODE_CODE_B:
          if (code < 96) {
            if (shiftUpperMode == upperMode) {
              result.append((char) (' ' + code));
            } else {
              result.append((char) (' ' + code + 128));
            }
            shiftUpperMode = false;
          } else {
            if (code != CODE_STOP) {
              lastCharacterWasPrintable = false;
            }
            switch (code) {
              case CODE_FNC_1:
                if (convertFNC1) {
                  if (result.length() == 0) {
                                                            result.append("]C1");
                  } else {
                                        result.append((char) 29);
                  }
                }
                break;
              case CODE_FNC_2:
              case CODE_FNC_3:
                                break;
              case CODE_FNC_4_B:
                if (!upperMode && shiftUpperMode) {
                  upperMode = true;
                  shiftUpperMode = false;
                } else if (upperMode && shiftUpperMode) {
                  upperMode = false;
                  shiftUpperMode = false;
                } else {
                  shiftUpperMode = true;
                }
                break;
              case CODE_SHIFT:
                isNextShifted = true;
                codeSet = CODE_CODE_A;
                break;
              case CODE_CODE_A:
                codeSet = CODE_CODE_A;
                break;
              case CODE_CODE_C:
                codeSet = CODE_CODE_C;
                break;
              case CODE_STOP:
                done = true;
                break;
            }
          }
          break;
        case CODE_CODE_C:
          if (code < 100) {
            if (code < 10) {
              result.append('0');
            }
            result.append(code);
          } else {
            if (code != CODE_STOP) {
              lastCharacterWasPrintable = false;
            }
            switch (code) {
              case CODE_FNC_1:
                if (convertFNC1) {
                  if (result.length() == 0) {
                                                            result.append("]C1");
                  } else {
                                        result.append((char) 29);
                  }
                }
                break;
              case CODE_CODE_A:
                codeSet = CODE_CODE_A;
                break;
              case CODE_CODE_B:
                codeSet = CODE_CODE_B;
                break;
              case CODE_STOP:
                done = true;
                break;
            }
          }
          break;
      }

            if (unshift) {
        codeSet = codeSet == CODE_CODE_A ? CODE_CODE_B : CODE_CODE_A;
      }

    }

    int lastPatternSize = nextStart - lastStart;

                nextStart = row.getNextUnset(nextStart);
    if (!row.isRange(nextStart,
                     Math.min(row.getSize(), nextStart + (nextStart - lastStart) / 2),
                     false)) {
      throw NotFoundException.getNotFoundInstance();
    }

        checksumTotal -= multiplier * lastCode;
        if (checksumTotal % 103 != lastCode) {
      throw ChecksumException.getChecksumInstance();
    }

        int resultLength = result.length();
    if (resultLength == 0) {
            throw NotFoundException.getNotFoundInstance();
    }

            if (resultLength > 0 && lastCharacterWasPrintable) {
      if (codeSet == CODE_CODE_C) {
        result.delete(resultLength - 2, resultLength);
      } else {
        result.delete(resultLength - 1, resultLength);
      }
    }

    float left = (startPatternInfo[1] + startPatternInfo[0]) / 2.0f;
    float right = lastStart + lastPatternSize / 2.0f;

    int rawCodesSize = rawCodes.size();
    byte[] rawBytes = new byte[rawCodesSize];
    for (int i = 0; i < rawCodesSize; i++) {
      rawBytes[i] = rawCodes.get(i);
    }

    return new Result(
        result.toString(),
        rawBytes,
        new ResultPoint[]{
            new ResultPoint(left, rowNumber),
            new ResultPoint(right, rowNumber)},
        BarcodeFormat.CODE_128);

  }

}
