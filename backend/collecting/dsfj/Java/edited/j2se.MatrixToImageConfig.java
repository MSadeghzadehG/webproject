

package com.google.zxing.client.j2se;

import java.awt.image.BufferedImage;


public final class MatrixToImageConfig {

  public static final int BLACK = 0xFF000000;
  public static final int WHITE = 0xFFFFFFFF;
  
  private final int onColor;
  private final int offColor;

  
  public MatrixToImageConfig() {
    this(BLACK, WHITE);
  }

  
  public MatrixToImageConfig(int onColor, int offColor) {
    this.onColor = onColor;
    this.offColor = offColor;
  }

  public int getPixelOnColor() {
    return onColor;
  }

  public int getPixelOffColor() {
    return offColor;
  }

  int getBufferedImageColorModel() {
    if (onColor == BLACK && offColor == WHITE) {
            return BufferedImage.TYPE_BYTE_BINARY;
    }
    if (hasTransparency(onColor) || hasTransparency(offColor)) {
            return BufferedImage.TYPE_INT_ARGB;
    }
        return BufferedImage.TYPE_INT_RGB;
  }

  private static boolean hasTransparency(int argb) {
    return (argb & 0xFF000000) != 0xFF000000;
  }

}
