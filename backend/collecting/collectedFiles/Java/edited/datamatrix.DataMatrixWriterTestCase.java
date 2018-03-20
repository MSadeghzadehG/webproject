

package com.google.zxing.datamatrix;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.datamatrix.encoder.SymbolShapeHint;
import org.junit.Assert;
import org.junit.Test;

import java.util.EnumMap;
import java.util.Map;


public final class DataMatrixWriterTestCase extends Assert {

  @Test
  public void testDataMatrixImageWriter() {

    Map<EncodeHintType,Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.DATA_MATRIX_SHAPE, SymbolShapeHint.FORCE_SQUARE);

    int bigEnough = 64;
    DataMatrixWriter writer = new DataMatrixWriter();
    BitMatrix matrix = writer.encode("Hello Google", BarcodeFormat.DATA_MATRIX, bigEnough, bigEnough, hints);
    assertNotNull(matrix);
    assertTrue(bigEnough >= matrix.getWidth());
    assertTrue(bigEnough >= matrix.getHeight());
  }

  @Test
  public void testDataMatrixWriter() {

    Map<EncodeHintType,Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.DATA_MATRIX_SHAPE, SymbolShapeHint.FORCE_SQUARE);

    int bigEnough = 14;
    DataMatrixWriter writer = new DataMatrixWriter();
    BitMatrix matrix = writer.encode("Hello Me", BarcodeFormat.DATA_MATRIX, bigEnough, bigEnough, hints);
    assertNotNull(matrix);
    assertEquals(bigEnough, matrix.getWidth());
    assertEquals(bigEnough, matrix.getHeight());
  }

  @Test
  public void testDataMatrixTooSmall() {
        int tooSmall = 8;
    DataMatrixWriter writer = new DataMatrixWriter();
    BitMatrix matrix = writer.encode("http:    assertNotNull(matrix);
    assertTrue(tooSmall < matrix.getWidth());
    assertTrue(tooSmall < matrix.getHeight());
  }

}
