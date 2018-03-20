

package com.google.zxing.qrcode;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.Writer;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.qrcode.decoder.ErrorCorrectionLevel;
import org.junit.Assert;
import org.junit.Test;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.EnumMap;
import java.util.Map;


public final class QRCodeWriterTestCase extends Assert {

  private static final Path BASE_IMAGE_PATH = Paths.get("src/test/resources/golden/qrcode/");

  private static BufferedImage loadImage(String fileName) throws IOException {
    Path file = BASE_IMAGE_PATH.resolve(fileName);
    if (!Files.exists(file)) {
            file = Paths.get("core/").resolve(BASE_IMAGE_PATH).resolve(fileName);
    }
    assertTrue("Please download and install test images, and run from the 'core' directory", Files.exists(file));
    return ImageIO.read(file.toFile());
  }

    private static BitMatrix createMatrixFromImage(BufferedImage image) {
    int width = image.getWidth();
    int height = image.getHeight();
    int[] pixels = new int[width * height];
    image.getRGB(0, 0, width, height, pixels, 0, width);

    BitMatrix matrix = new BitMatrix(width, height);
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int pixel = pixels[y * width + x];
        int luminance = (306 * ((pixel >> 16) & 0xFF) +
            601 * ((pixel >> 8) & 0xFF) +
            117 * (pixel & 0xFF)) >> 10;
        if (luminance <= 0x7F) {
          matrix.set(x, y);
        }
      }
    }
    return matrix;
  }

  @Test
  public void testQRCodeWriter() throws WriterException {
        int bigEnough = 256;
    Writer writer = new QRCodeWriter();
    BitMatrix matrix = writer.encode("http:        bigEnough, null);
    assertNotNull(matrix);
    assertEquals(bigEnough, matrix.getWidth());
    assertEquals(bigEnough, matrix.getHeight());

        int tooSmall = 20;
    matrix = writer.encode("http:        tooSmall, null);
    assertNotNull(matrix);
    assertTrue(tooSmall < matrix.getWidth());
    assertTrue(tooSmall < matrix.getHeight());

        int strangeWidth = 500;
    int strangeHeight = 100;
    matrix = writer.encode("http:        strangeHeight, null);
    assertNotNull(matrix);
    assertEquals(strangeWidth, matrix.getWidth());
    assertEquals(strangeHeight, matrix.getHeight());
  }

  private static void compareToGoldenFile(String contents,
                                          ErrorCorrectionLevel ecLevel,
                                          int resolution,
                                          String fileName) throws WriterException, IOException {

    BufferedImage image = loadImage(fileName);
    assertNotNull(image);
    BitMatrix goldenResult = createMatrixFromImage(image);
    assertNotNull(goldenResult);

    Map<EncodeHintType,Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.ERROR_CORRECTION, ecLevel);
    Writer writer = new QRCodeWriter();
    BitMatrix generatedResult = writer.encode(contents, BarcodeFormat.QR_CODE, resolution,
        resolution, hints);

    assertEquals(resolution, generatedResult.getWidth());
    assertEquals(resolution, generatedResult.getHeight());
    assertEquals(goldenResult, generatedResult);
  }

        @Test
  public void testRegressionTest() throws Exception {
    compareToGoldenFile("http:        "renderer-test-01.png");
  }

}
