

package com.google.zxing.oned;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.Writer;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;

import java.util.Map;


public abstract class OneDimensionalCodeWriter implements Writer {

  @Override
  public final BitMatrix encode(String contents, BarcodeFormat format, int width, int height)
      throws WriterException {
    return encode(contents, format, width, height, null);
  }

  
  @Override
  public BitMatrix encode(String contents,
                          BarcodeFormat format,
                          int width,
                          int height,
                          Map<EncodeHintType,?> hints) throws WriterException {
    if (contents.isEmpty()) {
      throw new IllegalArgumentException("Found empty contents");
    }

    if (width < 0 || height < 0) {
      throw new IllegalArgumentException("Negative size is not allowed. Input: "
                                             + width + 'x' + height);
    }

    int sidesMargin = getDefaultMargin();
    if (hints != null && hints.containsKey(EncodeHintType.MARGIN)) {
      sidesMargin = Integer.parseInt(hints.get(EncodeHintType.MARGIN).toString());
    }

    boolean[] code = encode(contents);
    return renderResult(code, width, height, sidesMargin);
  }

  
  private static BitMatrix renderResult(boolean[] code, int width, int height, int sidesMargin) {
    int inputWidth = code.length;
        int fullWidth = inputWidth + sidesMargin;
    int outputWidth = Math.max(width, fullWidth);
    int outputHeight = Math.max(1, height);

    int multiple = outputWidth / fullWidth;
    int leftPadding = (outputWidth - (inputWidth * multiple)) / 2;

    BitMatrix output = new BitMatrix(outputWidth, outputHeight);
    for (int inputX = 0, outputX = leftPadding; inputX < inputWidth; inputX++, outputX += multiple) {
      if (code[inputX]) {
        output.setRegion(outputX, 0, multiple, outputHeight);
      }
    }
    return output;
  }


  
  protected static int appendPattern(boolean[] target, int pos, int[] pattern, boolean startColor) {
    boolean color = startColor;
    int numAdded = 0;
    for (int len : pattern) {
      for (int j = 0; j < len; j++) {
        target[pos++] = color;
      }
      numAdded += len;
      color = !color;     }
    return numAdded;
  }

  public int getDefaultMargin() {
            return 10;
  }

  
  public abstract boolean[] encode(String contents);
}

