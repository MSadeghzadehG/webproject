

package com.google.zxing.oned;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;


public final class Code128Writer extends OneDimensionalCodeWriter {

  private static final int CODE_START_A = 103;
  private static final int CODE_START_B = 104;
  private static final int CODE_START_C = 105;
  private static final int CODE_CODE_A = 101;
  private static final int CODE_CODE_B = 100;
  private static final int CODE_CODE_C = 99;
  private static final int CODE_STOP = 106;

    private static final char ESCAPE_FNC_1 = '\u00f1';
  private static final char ESCAPE_FNC_2 = '\u00f2';
  private static final char ESCAPE_FNC_3 = '\u00f3';
  private static final char ESCAPE_FNC_4 = '\u00f4';

  private static final int CODE_FNC_1 = 102;     private static final int CODE_FNC_2 = 97;      private static final int CODE_FNC_3 = 96;      private static final int CODE_FNC_4_A = 101;   private static final int CODE_FNC_4_B = 100; 
    private enum CType {
    UNCODABLE,
    ONE_DIGIT,
    TWO_DIGITS,
    FNC_1
  }

  @Override
  public BitMatrix encode(String contents,
                          BarcodeFormat format,
                          int width,
                          int height,
                          Map<EncodeHintType,?> hints) throws WriterException {
    if (format != BarcodeFormat.CODE_128) {
      throw new IllegalArgumentException("Can only encode CODE_128, but got " + format);
    }
    return super.encode(contents, format, width, height, hints);
  }

  @Override
  public boolean[] encode(String contents) {
    int length = contents.length();
        if (length < 1 || length > 80) {
      throw new IllegalArgumentException(
          "Contents length should be between 1 and 80 characters, but got " + length);
    }
        for (int i = 0; i < length; i++) {
      char c = contents.charAt(i);
      switch (c) {
        case ESCAPE_FNC_1:
        case ESCAPE_FNC_2:
        case ESCAPE_FNC_3:
        case ESCAPE_FNC_4:
          break;
        default:
          if (c > 127) {
                        throw new IllegalArgumentException("Bad character in input: " + c);
          }
      }
    }

    Collection<int[]> patterns = new ArrayList<>();     int checkSum = 0;
    int checkWeight = 1;
    int codeSet = 0;     int position = 0; 
    while (position < length) {
            int newCodeSet = chooseCode(contents, position, codeSet);

            int patternIndex;
      if (newCodeSet == codeSet) {
                        switch (contents.charAt(position)) {
          case ESCAPE_FNC_1:
            patternIndex = CODE_FNC_1;
            break;
          case ESCAPE_FNC_2:
            patternIndex = CODE_FNC_2;
            break;
          case ESCAPE_FNC_3:
            patternIndex = CODE_FNC_3;
            break;
          case ESCAPE_FNC_4:
            if (codeSet == CODE_CODE_A) {
              patternIndex = CODE_FNC_4_A;
            } else {
              patternIndex = CODE_FNC_4_B;
            }
            break;
          default:
                        switch (codeSet) {
              case CODE_CODE_A:
                patternIndex = contents.charAt(position) - ' ';
                if (patternIndex < 0) {
                                    patternIndex += '`';
                }
                break;
              case CODE_CODE_B:
                patternIndex = contents.charAt(position) - ' ';
                break;
              default:
                                patternIndex = Integer.parseInt(contents.substring(position, position + 2));
                position++;                 break;
            }
        }
        position++;
      } else {
                        if (codeSet == 0) {
                    switch (newCodeSet) {
            case CODE_CODE_A:
              patternIndex = CODE_START_A;
              break;
            case CODE_CODE_B:
              patternIndex = CODE_START_B;
              break;
            default:
              patternIndex = CODE_START_C;
              break;
          }
        } else {
                    patternIndex = newCodeSet;
        }
        codeSet = newCodeSet;
      }

            patterns.add(Code128Reader.CODE_PATTERNS[patternIndex]);

            checkSum += patternIndex * checkWeight;
      if (position != 0) {
        checkWeight++;
      }
    }

        checkSum %= 103;
    patterns.add(Code128Reader.CODE_PATTERNS[checkSum]);

        patterns.add(Code128Reader.CODE_PATTERNS[CODE_STOP]);

        int codeWidth = 0;
    for (int[] pattern : patterns) {
      for (int width : pattern) {
        codeWidth += width;
      }
    }

        boolean[] result = new boolean[codeWidth];
    int pos = 0;
    for (int[] pattern : patterns) {
      pos += appendPattern(result, pos, pattern, true);
    }

    return result;
  }

  private static CType findCType(CharSequence value, int start) {
    int last = value.length();
    if (start >= last) {
      return CType.UNCODABLE;
    }
    char c = value.charAt(start);
    if (c == ESCAPE_FNC_1) {
      return CType.FNC_1;
    }
    if (c < '0' || c > '9') {
      return CType.UNCODABLE;
    }
    if (start + 1 >= last) {
      return CType.ONE_DIGIT;
    }
    c = value.charAt(start + 1);
    if (c < '0' || c > '9') {
      return CType.ONE_DIGIT;
    }
    return CType.TWO_DIGITS;
  }

  private static int chooseCode(CharSequence value, int start, int oldCode) {
    CType lookahead = findCType(value, start);
    if (lookahead == CType.ONE_DIGIT) {
       return CODE_CODE_B;
    }
    if (lookahead == CType.UNCODABLE) {
      if (start < value.length()) {
        char c = value.charAt(start);
        if (c < ' ' || (oldCode == CODE_CODE_A && c < '`')) {
                    return CODE_CODE_A;
        }
      }
      return CODE_CODE_B;     }
    if (oldCode == CODE_CODE_C) {       return CODE_CODE_C;
    }
    if (oldCode == CODE_CODE_B) {
      if (lookahead == CType.FNC_1) {
        return CODE_CODE_B;       }
            lookahead = findCType(value, start + 2);
      if (lookahead == CType.UNCODABLE || lookahead == CType.ONE_DIGIT) {
        return CODE_CODE_B;       }
      if (lookahead == CType.FNC_1) {         lookahead = findCType(value, start + 3);
        if (lookahead == CType.TWO_DIGITS) {           return CODE_CODE_C;
        } else {
          return CODE_CODE_B;         }
      }
                  int index = start + 4;
      while ((lookahead = findCType(value, index)) == CType.TWO_DIGITS) {
        index += 2;
      }
      if (lookahead == CType.ONE_DIGIT) {         return CODE_CODE_B;
      }
      return CODE_CODE_C;     }
        if (lookahead == CType.FNC_1) {       lookahead = findCType(value, start + 1);
    }
    if (lookahead == CType.TWO_DIGITS) {       return CODE_CODE_C;
    }
    return CODE_CODE_B;
  }

}
