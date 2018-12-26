

package com.google.zxing.oned;

import java.util.Map;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.FormatException;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;


public final class UPCEWriter extends UPCEANWriter {

  private static final int CODE_WIDTH = 3 +       (7 * 6) +       6; 
  @Override
  public BitMatrix encode(String contents,
                          BarcodeFormat format,
                          int width,
                          int height,
                          Map<EncodeHintType, ?> hints) throws WriterException {
    if (format != BarcodeFormat.UPC_E) {
      throw new IllegalArgumentException("Can only encode UPC_E, but got " + format);
    }

    return super.encode(contents, format, width, height, hints);
  }

  @Override
  public boolean[] encode(String contents) {
    int length = contents.length();
    switch (length) {
      case 7:
                int check;
        try {
          check = UPCEANReader.getStandardUPCEANChecksum(UPCEReader.convertUPCEtoUPCA(contents));
        } catch (FormatException fe) {
          throw new IllegalArgumentException(fe);
        }
        contents += check;
        break;
      case 8:
        try {
          if (!UPCEANReader.checkStandardUPCEANChecksum(contents)) {
            throw new IllegalArgumentException("Contents do not pass checksum");
          }
        } catch (FormatException ignored) {
          throw new IllegalArgumentException("Illegal contents");
        }
        break;
      default:
        throw new IllegalArgumentException(
            "Requested contents should be 8 digits long, but got " + length);
    }

    int firstDigit = Character.digit(contents.charAt(0), 10);
    if (firstDigit != 0 && firstDigit != 1) {
      throw new IllegalArgumentException("Number system must be 0 or 1");
    }

    int checkDigit = Character.digit(contents.charAt(7), 10);
    int parities = UPCEReader.NUMSYS_AND_CHECK_DIGIT_PATTERNS[firstDigit][checkDigit];
    boolean[] result = new boolean[CODE_WIDTH];
    int pos = 0;

    pos += appendPattern(result, pos, UPCEANReader.START_END_PATTERN, true);

    for (int i = 1; i <= 6; i++) {
      int digit = Character.digit(contents.charAt(i), 10);
      if ((parities >> (6 - i) & 1) == 1) {
        digit += 10;
      }
      pos += appendPattern(result, pos, UPCEANReader.L_AND_G_PATTERNS[digit], false);
    }

    appendPattern(result, pos, UPCEANReader.END_PATTERN, false);

    return result;
  }

}
