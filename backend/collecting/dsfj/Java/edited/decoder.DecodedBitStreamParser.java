

package com.google.zxing.qrcode.decoder;

import com.google.zxing.DecodeHintType;
import com.google.zxing.FormatException;
import com.google.zxing.common.BitSource;
import com.google.zxing.common.CharacterSetECI;
import com.google.zxing.common.DecoderResult;
import com.google.zxing.common.StringUtils;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;


final class DecodedBitStreamParser {

  
  private static final char[] ALPHANUMERIC_CHARS =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:".toCharArray();
  private static final int GB2312_SUBSET = 1;

  private DecodedBitStreamParser() {
  }

  static DecoderResult decode(byte[] bytes,
                              Version version,
                              ErrorCorrectionLevel ecLevel,
                              Map<DecodeHintType,?> hints) throws FormatException {
    BitSource bits = new BitSource(bytes);
    StringBuilder result = new StringBuilder(50);
    List<byte[]> byteSegments = new ArrayList<>(1);
    int symbolSequence = -1;
    int parityData = -1;

    try {
      CharacterSetECI currentCharacterSetECI = null;
      boolean fc1InEffect = false;
      Mode mode;
      do {
                if (bits.available() < 4) {
                    mode = Mode.TERMINATOR;
        } else {
          mode = Mode.forBits(bits.readBits(4));         }
        switch (mode) {
          case TERMINATOR:
            break;
          case FNC1_FIRST_POSITION:
          case FNC1_SECOND_POSITION:
                        fc1InEffect = true;
            break;
          case STRUCTURED_APPEND:
            if (bits.available() < 16) {
              throw FormatException.getFormatInstance();
            }
                                    symbolSequence = bits.readBits(8);
            parityData = bits.readBits(8);
            break;
          case ECI:
                        int value = parseECIValue(bits);
            currentCharacterSetECI = CharacterSetECI.getCharacterSetECIByValue(value);
            if (currentCharacterSetECI == null) {
              throw FormatException.getFormatInstance();
            }
            break;
          case HANZI:
                                    int subset = bits.readBits(4);
            int countHanzi = bits.readBits(mode.getCharacterCountBits(version));
            if (subset == GB2312_SUBSET) {
              decodeHanziSegment(bits, result, countHanzi);
            }
            break;
          default:
                                    int count = bits.readBits(mode.getCharacterCountBits(version));
            switch (mode) {
              case NUMERIC:
                decodeNumericSegment(bits, result, count);
                break;
              case ALPHANUMERIC:
                decodeAlphanumericSegment(bits, result, count, fc1InEffect);
                break;
              case BYTE:
                decodeByteSegment(bits, result, count, currentCharacterSetECI, byteSegments, hints);
                break;
              case KANJI:
                decodeKanjiSegment(bits, result, count);
                break;
              default:
                throw FormatException.getFormatInstance();
            }
            break;
        }
      } while (mode != Mode.TERMINATOR);
    } catch (IllegalArgumentException iae) {
            throw FormatException.getFormatInstance();
    }

    return new DecoderResult(bytes,
                             result.toString(),
                             byteSegments.isEmpty() ? null : byteSegments,
                             ecLevel == null ? null : ecLevel.toString(),
                             symbolSequence,
                             parityData);
  }

  
  private static void decodeHanziSegment(BitSource bits,
                                         StringBuilder result,
                                         int count) throws FormatException {
        if (count * 13 > bits.available()) {
      throw FormatException.getFormatInstance();
    }

            byte[] buffer = new byte[2 * count];
    int offset = 0;
    while (count > 0) {
            int twoBytes = bits.readBits(13);
      int assembledTwoBytes = ((twoBytes / 0x060) << 8) | (twoBytes % 0x060);
      if (assembledTwoBytes < 0x003BF) {
                assembledTwoBytes += 0x0A1A1;
      } else {
                assembledTwoBytes += 0x0A6A1;
      }
      buffer[offset] = (byte) ((assembledTwoBytes >> 8) & 0xFF);
      buffer[offset + 1] = (byte) (assembledTwoBytes & 0xFF);
      offset += 2;
      count--;
    }

    try {
      result.append(new String(buffer, StringUtils.GB2312));
    } catch (UnsupportedEncodingException ignored) {
      throw FormatException.getFormatInstance();
    }
  }

  private static void decodeKanjiSegment(BitSource bits,
                                         StringBuilder result,
                                         int count) throws FormatException {
        if (count * 13 > bits.available()) {
      throw FormatException.getFormatInstance();
    }

            byte[] buffer = new byte[2 * count];
    int offset = 0;
    while (count > 0) {
            int twoBytes = bits.readBits(13);
      int assembledTwoBytes = ((twoBytes / 0x0C0) << 8) | (twoBytes % 0x0C0);
      if (assembledTwoBytes < 0x01F00) {
                assembledTwoBytes += 0x08140;
      } else {
                assembledTwoBytes += 0x0C140;
      }
      buffer[offset] = (byte) (assembledTwoBytes >> 8);
      buffer[offset + 1] = (byte) assembledTwoBytes;
      offset += 2;
      count--;
    }
        try {
      result.append(new String(buffer, StringUtils.SHIFT_JIS));
    } catch (UnsupportedEncodingException ignored) {
      throw FormatException.getFormatInstance();
    }
  }

  private static void decodeByteSegment(BitSource bits,
                                        StringBuilder result,
                                        int count,
                                        CharacterSetECI currentCharacterSetECI,
                                        Collection<byte[]> byteSegments,
                                        Map<DecodeHintType,?> hints) throws FormatException {
        if (8 * count > bits.available()) {
      throw FormatException.getFormatInstance();
    }

    byte[] readBytes = new byte[count];
    for (int i = 0; i < count; i++) {
      readBytes[i] = (byte) bits.readBits(8);
    }
    String encoding;
    if (currentCharacterSetECI == null) {
                                    encoding = StringUtils.guessEncoding(readBytes, hints);
    } else {
      encoding = currentCharacterSetECI.name();
    }
    try {
      result.append(new String(readBytes, encoding));
    } catch (UnsupportedEncodingException ignored) {
      throw FormatException.getFormatInstance();
    }
    byteSegments.add(readBytes);
  }

  private static char toAlphaNumericChar(int value) throws FormatException {
    if (value >= ALPHANUMERIC_CHARS.length) {
      throw FormatException.getFormatInstance();
    }
    return ALPHANUMERIC_CHARS[value];
  }

  private static void decodeAlphanumericSegment(BitSource bits,
                                                StringBuilder result,
                                                int count,
                                                boolean fc1InEffect) throws FormatException {
        int start = result.length();
    while (count > 1) {
      if (bits.available() < 11) {
        throw FormatException.getFormatInstance();
      }
      int nextTwoCharsBits = bits.readBits(11);
      result.append(toAlphaNumericChar(nextTwoCharsBits / 45));
      result.append(toAlphaNumericChar(nextTwoCharsBits % 45));
      count -= 2;
    }
    if (count == 1) {
            if (bits.available() < 6) {
        throw FormatException.getFormatInstance();
      }
      result.append(toAlphaNumericChar(bits.readBits(6)));
    }
        if (fc1InEffect) {
            for (int i = start; i < result.length(); i++) {
        if (result.charAt(i) == '%') {
          if (i < result.length() - 1 && result.charAt(i + 1) == '%') {
                        result.deleteCharAt(i + 1);
          } else {
                        result.setCharAt(i, (char) 0x1D);
          }
        }
      }
    }
  }

  private static void decodeNumericSegment(BitSource bits,
                                           StringBuilder result,
                                           int count) throws FormatException {
        while (count >= 3) {
            if (bits.available() < 10) {
        throw FormatException.getFormatInstance();
      }
      int threeDigitsBits = bits.readBits(10);
      if (threeDigitsBits >= 1000) {
        throw FormatException.getFormatInstance();
      }
      result.append(toAlphaNumericChar(threeDigitsBits / 100));
      result.append(toAlphaNumericChar((threeDigitsBits / 10) % 10));
      result.append(toAlphaNumericChar(threeDigitsBits % 10));
      count -= 3;
    }
    if (count == 2) {
            if (bits.available() < 7) {
        throw FormatException.getFormatInstance();
      }
      int twoDigitsBits = bits.readBits(7);
      if (twoDigitsBits >= 100) {
        throw FormatException.getFormatInstance();
      }
      result.append(toAlphaNumericChar(twoDigitsBits / 10));
      result.append(toAlphaNumericChar(twoDigitsBits % 10));
    } else if (count == 1) {
            if (bits.available() < 4) {
        throw FormatException.getFormatInstance();
      }
      int digitBits = bits.readBits(4);
      if (digitBits >= 10) {
        throw FormatException.getFormatInstance();
      }
      result.append(toAlphaNumericChar(digitBits));
    }
  }

  private static int parseECIValue(BitSource bits) throws FormatException {
    int firstByte = bits.readBits(8);
    if ((firstByte & 0x80) == 0) {
            return firstByte & 0x7F;
    }
    if ((firstByte & 0xC0) == 0x80) {
            int secondByte = bits.readBits(8);
      return ((firstByte & 0x3F) << 8) | secondByte;
    }
    if ((firstByte & 0xE0) == 0xC0) {
            int secondThirdBytes = bits.readBits(16);
      return ((firstByte & 0x1F) << 16) | secondThirdBytes;
    }
    throw FormatException.getFormatInstance();
  }

}
