

package com.google.zxing.web;

import com.google.zxing.qrcode.decoder.ErrorCorrectionLevel;

import java.nio.charset.Charset;


final class ChartServletRequestParameters {

  private final int width;
  private final int height;
  private final Charset outputEncoding;
  private final ErrorCorrectionLevel ecLevel;
  private final int margin;
  private final String text;

  ChartServletRequestParameters(int width,
                                int height,
                                Charset outputEncoding,
                                ErrorCorrectionLevel ecLevel,
                                int margin,
                                String text) {
    this.width = width;
    this.height = height;
    this.outputEncoding = outputEncoding;
    this.ecLevel = ecLevel;
    this.margin = margin;
    this.text = text;
  }

  int getWidth() {
    return width;
  }

  int getHeight() {
    return height;
  }

  Charset getOutputEncoding() {
    return outputEncoding;
  }

  ErrorCorrectionLevel getEcLevel() {
    return ecLevel;
  }

  int getMargin() {
    return margin;
  }

  String getText() {
    return text;
  }

}
