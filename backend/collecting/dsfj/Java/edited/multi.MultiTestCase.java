

package com.google.zxing.multi;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.nio.file.Path;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.BinaryBitmap;
import com.google.zxing.BufferedImageLuminanceSource;
import com.google.zxing.LuminanceSource;
import com.google.zxing.MultiFormatReader;
import com.google.zxing.Result;
import com.google.zxing.common.AbstractBlackBoxTestCase;
import com.google.zxing.common.HybridBinarizer;
import org.junit.Assert;
import org.junit.Test;

public final class MultiTestCase extends Assert {

  @Test
  public void testMulti() throws Exception {
        Path testBase = AbstractBlackBoxTestCase.buildTestBase("src/test/resources/blackbox/multi-1");

    Path testImage = testBase.resolve("1.png");
    BufferedImage image = ImageIO.read(testImage.toFile());
    LuminanceSource source = new BufferedImageLuminanceSource(image);
    BinaryBitmap bitmap = new BinaryBitmap(new HybridBinarizer(source));

    MultipleBarcodeReader reader = new GenericMultipleBarcodeReader(new MultiFormatReader());
    Result[] results = reader.decodeMultiple(bitmap);
    assertNotNull(results);
    assertEquals(2, results.length);

    assertEquals("031415926531", results[0].getText());
    assertEquals(BarcodeFormat.UPC_A, results[0].getBarcodeFormat());

    assertEquals("www.airtable.com/jobs", results[1].getText());
    assertEquals(BarcodeFormat.QR_CODE, results[1].getBarcodeFormat());
  }

}
