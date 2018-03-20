



package com.google.zxing.oned.rss.expanded;

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

import javax.imageio.ImageIO;

import com.google.zxing.BinaryBitmap;
import com.google.zxing.BufferedImageLuminanceSource;
import com.google.zxing.NotFoundException;
import com.google.zxing.common.AbstractBlackBoxTestCase;
import com.google.zxing.common.BitArray;
import com.google.zxing.common.GlobalHistogramBinarizer;
import com.google.zxing.oned.rss.DataCharacter;
import com.google.zxing.oned.rss.FinderPattern;

import org.junit.Assert;
import org.junit.Test;


public final class RSSExpandedInternalTestCase extends Assert {

  @Test
  public void testFindFinderPatterns() throws Exception {
    BufferedImage image = readImage("2.png");
    BinaryBitmap binaryMap = new BinaryBitmap(new GlobalHistogramBinarizer(new BufferedImageLuminanceSource(image)));
    int rowNumber = binaryMap.getHeight() / 2;
    BitArray row = binaryMap.getBlackRow(rowNumber, null);
    List<ExpandedPair> previousPairs = new ArrayList<>();

    RSSExpandedReader rssExpandedReader = new RSSExpandedReader();
    ExpandedPair pair1 = rssExpandedReader.retrieveNextPair(row, previousPairs, rowNumber);
    previousPairs.add(pair1);
    FinderPattern finderPattern = pair1.getFinderPattern();
    assertNotNull(finderPattern);
    assertEquals(0, finderPattern.getValue());

    ExpandedPair pair2 = rssExpandedReader.retrieveNextPair(row, previousPairs, rowNumber);
    previousPairs.add(pair2);
    finderPattern = pair2.getFinderPattern();
    assertNotNull(finderPattern);
    assertEquals(1, finderPattern.getValue());

    ExpandedPair pair3 = rssExpandedReader.retrieveNextPair(row, previousPairs, rowNumber);
    previousPairs.add(pair3);
    finderPattern = pair3.getFinderPattern();
    assertNotNull(finderPattern);
    assertEquals(1, finderPattern.getValue());

    try{
      rssExpandedReader.retrieveNextPair(row, previousPairs, rowNumber);
            fail(NotFoundException.class.getName() + " expected");
    } catch (NotFoundException nfe) {
          }
  }

  @Test
  public void testRetrieveNextPairPatterns() throws Exception {
    BufferedImage image = readImage("3.png");
    BinaryBitmap binaryMap = new BinaryBitmap(new GlobalHistogramBinarizer(new BufferedImageLuminanceSource(image)));
    int rowNumber = binaryMap.getHeight() / 2;
    BitArray row = binaryMap.getBlackRow(rowNumber, null);
    List<ExpandedPair> previousPairs = new ArrayList<>();

    RSSExpandedReader rssExpandedReader = new RSSExpandedReader();
    ExpandedPair pair1 = rssExpandedReader.retrieveNextPair(row, previousPairs, rowNumber);
    previousPairs.add(pair1);
    FinderPattern finderPattern = pair1.getFinderPattern();
    assertNotNull(finderPattern);
    assertEquals(0, finderPattern.getValue());

    ExpandedPair pair2 = rssExpandedReader.retrieveNextPair(row, previousPairs, rowNumber);
    previousPairs.add(pair2);
    finderPattern = pair2.getFinderPattern();
    assertNotNull(finderPattern);
    assertEquals(0, finderPattern.getValue());
  }

  @Test
  public void testDecodeCheckCharacter() throws Exception {
    BufferedImage image = readImage("3.png");
    BinaryBitmap binaryMap = new BinaryBitmap(new GlobalHistogramBinarizer(new BufferedImageLuminanceSource(image)));
    BitArray row = binaryMap.getBlackRow(binaryMap.getHeight() / 2, null);

    int[] startEnd = {145, 243};    int value = 0;    FinderPattern finderPatternA1 = new FinderPattern(value, startEnd, startEnd[0], startEnd[1], image.getHeight() / 2);
        RSSExpandedReader rssExpandedReader = new RSSExpandedReader();
    DataCharacter dataCharacter = rssExpandedReader.decodeDataCharacter(row, finderPatternA1, true, true);

    assertEquals(98, dataCharacter.getValue());
  }

  @Test
  public void testDecodeDataCharacter() throws Exception {
    BufferedImage image = readImage("3.png");
    BinaryBitmap binaryMap = new BinaryBitmap(new GlobalHistogramBinarizer(new BufferedImageLuminanceSource(image)));
    BitArray row = binaryMap.getBlackRow(binaryMap.getHeight() / 2, null);

    int[] startEnd = {145, 243};    int value = 0;     FinderPattern finderPatternA1 = new FinderPattern(value, startEnd, startEnd[0], startEnd[1], image.getHeight() / 2);
        RSSExpandedReader rssExpandedReader = new RSSExpandedReader();
    DataCharacter dataCharacter = rssExpandedReader.decodeDataCharacter(row, finderPatternA1, true, false);

    assertEquals(19, dataCharacter.getValue());
    assertEquals(1007, dataCharacter.getChecksumPortion());
  }

  private static BufferedImage readImage(String fileName) throws IOException {
    Path path = AbstractBlackBoxTestCase.buildTestBase("src/test/resources/blackbox/rssexpanded-1/").resolve(fileName);
    return ImageIO.read(path.toFile());
  }

}
