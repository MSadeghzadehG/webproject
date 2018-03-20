

package com.google.zxing.multi.qrcode;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.nio.file.Path;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.BinaryBitmap;
import com.google.zxing.BufferedImageLuminanceSource;
import com.google.zxing.LuminanceSource;
import com.google.zxing.Result;
import com.google.zxing.ResultMetadataType;
import com.google.zxing.common.AbstractBlackBoxTestCase;
import com.google.zxing.common.HybridBinarizer;
import com.google.zxing.multi.MultipleBarcodeReader;
import org.junit.Assert;
import org.junit.Test;

public final class MultiQRCodeTestCase extends Assert {

  @Test
  public void testMultiQRCodes() throws Exception {
        Path testBase = AbstractBlackBoxTestCase.buildTestBase("src/test/resources/blackbox/multi-qrcode-1");

    Path testImage = testBase.resolve("1.png");
    BufferedImage image = ImageIO.read(testImage.toFile());
    LuminanceSource source = new BufferedImageLuminanceSource(image);
    BinaryBitmap bitmap = new BinaryBitmap(new HybridBinarizer(source));

    MultipleBarcodeReader reader = new QRCodeMultiReader();
    Result[] results = reader.decodeMultiple(bitmap);
    assertNotNull(results);
    assertEquals(4, results.length);

    Set<String> barcodeContents = new HashSet<>();
    for (Result result : results) {
      barcodeContents.add(result.getText());
      assertEquals(BarcodeFormat.QR_CODE, result.getBarcodeFormat());
      Map<ResultMetadataType,Object> metadata = result.getResultMetadata();
      assertNotNull(metadata);
    }
    Set<String> expectedContents = new HashSet<>();
    expectedContents.add("You earned the class a 5 MINUTE DANCE PARTY!!  Awesome!  Way to go!  Let's boogie!");
    expectedContents.add("You earned the class 5 EXTRA MINUTES OF RECESS!!  Fabulous!!  Way to go!!");
    expectedContents.add("You get to SIT AT MRS. SIGMON'S DESK FOR A DAY!!  Awesome!!  Way to go!! Guess I better clean up! :)");
    expectedContents.add("You get to CREATE OUR JOURNAL PROMPT FOR THE DAY!  Yay!  Way to go!  ");
    assertEquals(expectedContents, barcodeContents);
  }

}
