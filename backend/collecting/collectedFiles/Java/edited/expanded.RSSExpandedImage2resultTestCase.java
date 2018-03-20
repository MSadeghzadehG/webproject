

package com.google.zxing.oned.rss.expanded;

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.nio.file.Path;
import java.util.HashMap;

import javax.imageio.ImageIO;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.BinaryBitmap;
import com.google.zxing.BufferedImageLuminanceSource;
import com.google.zxing.NotFoundException;
import com.google.zxing.ReaderException;
import com.google.zxing.Result;
import com.google.zxing.client.result.ExpandedProductParsedResult;
import com.google.zxing.client.result.ParsedResult;
import com.google.zxing.client.result.ResultParser;
import com.google.zxing.common.AbstractBlackBoxTestCase;
import com.google.zxing.common.BitArray;
import com.google.zxing.common.GlobalHistogramBinarizer;

import org.junit.Assert;
import org.junit.Test;


public final class RSSExpandedImage2resultTestCase extends Assert {

  @Test
  public void testDecodeRow2result_2() throws Exception {
        ExpandedProductParsedResult expected =
        new ExpandedProductParsedResult("(01)90012345678908(3103)001750",
                                        "90012345678908",
                                        null, null, null, null, null, null,
                                        "001750",
                                        ExpandedProductParsedResult.KILOGRAM,
                                        "3", null, null, null, new HashMap<String,String>());

    assertCorrectImage2result("2.png", expected);
  }

  private static void assertCorrectImage2result(String fileName, ExpandedProductParsedResult expected)
      throws IOException, NotFoundException {
    Path path = AbstractBlackBoxTestCase.buildTestBase("src/test/resources/blackbox/rssexpanded-1/").resolve(fileName);

    BufferedImage image = ImageIO.read(path.toFile());
    BinaryBitmap binaryMap = new BinaryBitmap(new GlobalHistogramBinarizer(new BufferedImageLuminanceSource(image)));
    int rowNumber = binaryMap.getHeight() / 2;
    BitArray row = binaryMap.getBlackRow(rowNumber, null);

    Result theResult;
    try {
      RSSExpandedReader rssExpandedReader = new RSSExpandedReader();
      theResult = rssExpandedReader.decodeRow(rowNumber, row, null);
    } catch (ReaderException re) {
      fail(re.toString());
      return;
    }

    assertSame(BarcodeFormat.RSS_EXPANDED, theResult.getBarcodeFormat());

    ParsedResult result = ResultParser.parseResult(theResult);

    assertEquals(expected, result);
  }

}
