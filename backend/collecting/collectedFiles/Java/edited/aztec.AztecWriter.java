

package com.google.zxing.aztec;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.Writer;
import com.google.zxing.aztec.encoder.AztecCode;
import com.google.zxing.aztec.encoder.Encoder;
import com.google.zxing.common.BitMatrix;

import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.Map;


public final class AztecWriter implements Writer {

  @Override
  public BitMatrix encode(String contents, BarcodeFormat format, int width, int height) {
    return encode(contents, format, width, height, null);
  }

  @Override
  public BitMatrix encode(String contents, BarcodeFormat format, int width, int height, Map<EncodeHintType,?> hints) {
    Charset charset = StandardCharsets.ISO_8859_1;
    int eccPercent = Encoder.DEFAULT_EC_PERCENT;
    int layers = Encoder.DEFAULT_AZTEC_LAYERS;
    if (hints != null) {
      if (hints.containsKey(EncodeHintType.CHARACTER_SET)) {
        charset = Charset.forName(hints.get(EncodeHintType.CHARACTER_SET).toString());
      }
      if (hints.containsKey(EncodeHintType.ERROR_CORRECTION)) {
        eccPercent = Integer.parseInt(hints.get(EncodeHintType.ERROR_CORRECTION).toString());
      }
      if (hints.containsKey(EncodeHintType.AZTEC_LAYERS)) {
        layers = Integer.parseInt(hints.get(EncodeHintType.AZTEC_LAYERS).toString());
      }
    }
    return encode(contents, format, width, height, charset, eccPercent, layers);
  }

  private static BitMatrix encode(String contents, BarcodeFormat format,
                                  int width, int height,
                                  Charset charset, int eccPercent, int layers) {
    if (format != BarcodeFormat.AZTEC) {
      throw new IllegalArgumentException("Can only encode AZTEC, but got " + format);
    }
    AztecCode aztec = Encoder.encode(contents.getBytes(charset), eccPercent, layers);
    return renderResult(aztec, width, height);
  }

  private static BitMatrix renderResult(AztecCode code, int width, int height) {
    BitMatrix input = code.getMatrix();
    if (input == null) {
      throw new IllegalStateException();
    }
    int inputWidth = input.getWidth();
    int inputHeight = input.getHeight();
    int outputWidth = Math.max(width, inputWidth);
    int outputHeight = Math.max(height, inputHeight);

    int multiple = Math.min(outputWidth / inputWidth, outputHeight / inputHeight);
    int leftPadding = (outputWidth - (inputWidth * multiple)) / 2;
    int topPadding = (outputHeight - (inputHeight * multiple)) / 2;

    BitMatrix output = new BitMatrix(outputWidth, outputHeight);

    for (int inputY = 0, outputY = topPadding; inputY < inputHeight; inputY++, outputY += multiple) {
            for (int inputX = 0, outputX = leftPadding; inputX < inputWidth; inputX++, outputX += multiple) {
        if (input.get(inputX, inputY)) {
          output.setRegion(outputX, outputY, multiple, multiple);
        }
      }
    }
    return output;
  }
}
