



package com.google.zxing.oned.rss.expanded;

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.nio.file.Path;

import javax.imageio.ImageIO;

import com.google.zxing.BinaryBitmap;
import com.google.zxing.BufferedImageLuminanceSource;
import com.google.zxing.common.AbstractBlackBoxTestCase;
import com.google.zxing.common.GlobalHistogramBinarizer;

final class TestCaseUtil {

  private TestCaseUtil() {
  }

  private static BufferedImage getBufferedImage(String path) throws IOException {
    Path file = AbstractBlackBoxTestCase.buildTestBase(path);
    return ImageIO.read(file.toFile());
  }

  static BinaryBitmap getBinaryBitmap(String path) throws IOException {
    BufferedImage bufferedImage = getBufferedImage(path);
    BufferedImageLuminanceSource luminanceSource = new BufferedImageLuminanceSource(bufferedImage);
    return new BinaryBitmap(new GlobalHistogramBinarizer(luminanceSource));
  }

}
