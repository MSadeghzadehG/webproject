

package com.google.zxing.oned;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.FormatException;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;

import java.util.Map;


public final class EAN8Writer extends UPCEANWriter {

  private static final int CODE_WIDTH = 3 +       (7 * 4) +       5 +       (7 * 4) +       3; 
  @Override
  public BitMatrix encode(String contents,
                          BarcodeFormat format,
                          int width,
                          int height,
                          Map<EncodeHintType,?> hints) throws WriterException {
    if (format != BarcodeFormat.EAN_8) {
      throw new IllegalArgumentException("Can only encode EAN_8, but got "
          + format);
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
          check = UPCEANReader.getStandardUPCEANChecksum(contents);
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

    boolean[] result = new boolean[CODE_WIDTH];
    int pos = 0;

    pos += appendPattern(result, pos, UPCEANReader.START_END_PATTERN, true);

    for (int i = 0; i <= 3; i++) {
      int digit = Character.digit(contents.charAt(i), 10);
      pos += appendPattern(result, pos, UPCEANReader.L_PATTERNS[digit], false);
    }

    pos += appendPattern(result, pos, UPCEANReader.MIDDLE_PATTERN, false);

    for (int i = 4; i <= 7; i++) {
      int digit = Character.digit(contents.charAt(i), 10);
      pos += appendPattern(result, pos, UPCEANReader.L_PATTERNS[digit], true);
    }
    appendPattern(result, pos, UPCEANReader.START_END_PATTERN, true);

    return result;
  }

}
