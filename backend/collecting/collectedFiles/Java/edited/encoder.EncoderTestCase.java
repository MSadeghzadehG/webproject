

package com.google.zxing.qrcode.encoder;

import com.google.zxing.EncodeHintType;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitArray;
import com.google.zxing.qrcode.decoder.ErrorCorrectionLevel;
import com.google.zxing.qrcode.decoder.Mode;
import com.google.zxing.qrcode.decoder.Version;

import org.junit.Assert;
import org.junit.Test;

import java.io.UnsupportedEncodingException;
import java.util.EnumMap;
import java.util.Map;


public final class EncoderTestCase extends Assert {

  @Test
  public void testGetAlphanumericCode() {
        for (int i = 0; i < 10; ++i) {
      assertEquals(i, Encoder.getAlphanumericCode('0' + i));
    }

        for (int i = 10; i < 36; ++i) {
      assertEquals(i, Encoder.getAlphanumericCode('A' + i - 10));
    }

        assertEquals(36, Encoder.getAlphanumericCode(' '));
    assertEquals(37, Encoder.getAlphanumericCode('$'));
    assertEquals(38, Encoder.getAlphanumericCode('%'));
    assertEquals(39, Encoder.getAlphanumericCode('*'));
    assertEquals(40, Encoder.getAlphanumericCode('+'));
    assertEquals(41, Encoder.getAlphanumericCode('-'));
    assertEquals(42, Encoder.getAlphanumericCode('.'));
    assertEquals(43, Encoder.getAlphanumericCode('/'));
    assertEquals(44, Encoder.getAlphanumericCode(':'));

        assertEquals(-1, Encoder.getAlphanumericCode('a'));
    assertEquals(-1, Encoder.getAlphanumericCode('#'));
    assertEquals(-1, Encoder.getAlphanumericCode('\0'));
  }

  @Test
  public void testChooseMode() throws WriterException {
        assertSame(Mode.NUMERIC, Encoder.chooseMode("0"));
    assertSame(Mode.NUMERIC, Encoder.chooseMode("0123456789"));
        assertSame(Mode.ALPHANUMERIC, Encoder.chooseMode("A"));
    assertSame(Mode.ALPHANUMERIC,
               Encoder.chooseMode("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:"));
        assertSame(Mode.BYTE, Encoder.chooseMode("a"));
    assertSame(Mode.BYTE, Encoder.chooseMode("#"));
    assertSame(Mode.BYTE, Encoder.chooseMode(""));
            
        assertSame(Mode.BYTE,
               Encoder.chooseMode(shiftJISString(new byte[]{0x8, 0xa, 0x8, 0xa, 0x8, 0xa, 0x8, (byte) 0xa6})));

        assertSame(Mode.BYTE, Encoder.chooseMode(shiftJISString(new byte[]{0x9, 0xf, 0x9, 0x7b})));

        assertSame(Mode.BYTE, Encoder.chooseMode(shiftJISString(new byte[]{0xe, 0x4, 0x9, 0x5, 0x9, 0x61})));
  }

  @Test
  public void testEncode() throws WriterException {
    QRCode qrCode = Encoder.encode("ABCDEF", ErrorCorrectionLevel.H);
    String expected =
      "<<\n" +
          " mode: ALPHANUMERIC\n" +
          " ecLevel: H\n" +
          " version: 1\n" +
          " maskPattern: 4\n" +
          " matrix:\n" +
          " 1 1 1 1 1 1 1 0 0 1 0 1 0 0 1 1 1 1 1 1 1\n" +
          " 1 0 0 0 0 0 1 0 1 0 1 0 1 0 1 0 0 0 0 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 0 0 0 0 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 1 0 0 1 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 1 0 1 0 0 1 0 1 1 1 0 1\n" +
          " 1 0 0 0 0 0 1 0 1 0 0 1 1 0 1 0 0 0 0 0 1\n" +
          " 1 1 1 1 1 1 1 0 1 0 1 0 1 0 1 1 1 1 1 1 1\n" +
          " 0 0 0 0 0 0 0 0 1 0 0 0 1 0 0 0 0 0 0 0 0\n" +
          " 0 0 0 0 1 1 1 1 0 1 1 0 1 0 1 1 0 0 0 1 0\n" +
          " 0 0 0 0 1 1 0 1 1 1 0 0 1 1 1 1 0 1 1 0 1\n" +
          " 1 0 0 0 0 1 1 0 0 1 0 1 0 0 0 1 1 1 0 1 1\n" +
          " 1 0 0 1 1 1 0 0 1 1 1 1 0 0 0 0 1 0 0 0 0\n" +
          " 0 1 1 1 1 1 1 0 1 0 1 0 1 1 1 0 0 1 1 0 0\n" +
          " 0 0 0 0 0 0 0 0 1 1 0 0 0 1 1 0 0 0 1 0 1\n" +
          " 1 1 1 1 1 1 1 0 1 1 1 1 0 0 0 0 0 1 1 0 0\n" +
          " 1 0 0 0 0 0 1 0 1 1 0 1 0 0 0 1 0 1 1 1 1\n" +
          " 1 0 1 1 1 0 1 0 1 0 0 1 0 0 0 1 1 0 0 1 1\n" +
          " 1 0 1 1 1 0 1 0 0 0 1 1 0 1 0 0 0 0 1 1 1\n" +
          " 1 0 1 1 1 0 1 0 0 1 0 1 0 0 0 1 1 0 0 0 0\n" +
          " 1 0 0 0 0 0 1 0 0 1 0 0 1 0 0 1 1 0 0 0 1\n" +
          " 1 1 1 1 1 1 1 0 0 0 1 0 0 1 0 0 0 0 1 1 1\n" +
          ">>\n";
    assertEquals(expected, qrCode.toString());
  }
  
  @Test
  public void testEncodeWithVersion() throws WriterException {
    Map<EncodeHintType, Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.QR_VERSION, 7);
    QRCode qrCode = Encoder.encode("ABCDEF", ErrorCorrectionLevel.H, hints);
    assertTrue(qrCode.toString().contains(" version: 7\n"));
  }
  
  @Test(expected = WriterException.class)
  public void testEncodeWithVersionTooSmall() throws WriterException {
    Map<EncodeHintType, Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.QR_VERSION, 3);
    Encoder.encode("THISMESSAGEISTOOLONGFORAQRCODEVERSION3", ErrorCorrectionLevel.H, hints);
  }

  @Test
  public void testSimpleUTF8ECI() throws WriterException {
    Map<EncodeHintType,Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.CHARACTER_SET, "UTF8");
    QRCode qrCode = Encoder.encode("hello", ErrorCorrectionLevel.H, hints);
    String expected =
      "<<\n" +
          " mode: BYTE\n" +
          " ecLevel: H\n" +
          " version: 1\n" +
          " maskPattern: 6\n" +
          " matrix:\n" +
          " 1 1 1 1 1 1 1 0 0 0 1 1 0 0 1 1 1 1 1 1 1\n" +
          " 1 0 0 0 0 0 1 0 0 0 1 1 0 0 1 0 0 0 0 0 1\n" +
          " 1 0 1 1 1 0 1 0 1 0 0 1 1 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 1 0 0 0 1 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 1 1 0 0 0 1 0 1 1 1 0 1\n" +
          " 1 0 0 0 0 0 1 0 0 0 0 1 0 0 1 0 0 0 0 0 1\n" +
          " 1 1 1 1 1 1 1 0 1 0 1 0 1 0 1 1 1 1 1 1 1\n" +
          " 0 0 0 0 0 0 0 0 0 1 1 1 1 0 0 0 0 0 0 0 0\n" +
          " 0 0 0 1 1 0 1 1 0 0 0 0 1 0 0 0 0 1 1 0 0\n" +
          " 0 0 0 0 0 0 0 0 1 1 0 1 0 0 1 0 1 1 1 1 1\n" +
          " 1 1 0 0 0 1 1 1 0 0 0 1 1 0 0 1 0 1 0 1 1\n" +
          " 0 0 0 0 1 1 0 0 1 0 0 0 0 0 1 0 1 1 0 0 0\n" +
          " 0 1 1 0 0 1 1 0 0 1 1 1 0 1 1 1 1 1 1 1 1\n" +
          " 0 0 0 0 0 0 0 0 1 1 1 0 1 1 1 1 1 1 1 1 1\n" +
          " 1 1 1 1 1 1 1 0 1 0 1 0 0 0 1 0 0 0 0 0 0\n" +
          " 1 0 0 0 0 0 1 0 0 1 0 0 0 1 0 0 0 1 1 0 0\n" +
          " 1 0 1 1 1 0 1 0 1 0 0 0 1 0 1 0 0 0 1 0 0\n" +
          " 1 0 1 1 1 0 1 0 1 1 1 1 0 1 0 0 1 0 1 1 0\n" +
          " 1 0 1 1 1 0 1 0 0 1 1 1 0 0 1 0 0 1 0 1 1\n" +
          " 1 0 0 0 0 0 1 0 0 0 0 0 0 1 1 0 1 1 0 0 0\n" +
          " 1 1 1 1 1 1 1 0 0 0 0 1 0 1 0 0 1 0 1 0 0\n" +
          ">>\n";
    assertEquals(expected, qrCode.toString());
  }

  @Test
  public void testEncodeKanjiMode() throws WriterException {
    Map<EncodeHintType,Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.CHARACTER_SET, "Shift_JIS");
        QRCode qrCode = Encoder.encode("\u65e5\u672c", ErrorCorrectionLevel.M, hints);
    String expected =
      "<<\n" +
          " mode: KANJI\n" +
          " ecLevel: M\n" +
          " version: 1\n" +
          " maskPattern: 0\n" +
          " matrix:\n" +
          " 1 1 1 1 1 1 1 0 0 1 0 1 0 0 1 1 1 1 1 1 1\n" +
          " 1 0 0 0 0 0 1 0 1 1 0 0 0 0 1 0 0 0 0 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 1 1 1 1 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 0 0 0 1 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 1 1 1 1 1 0 1 0 1 1 1 0 1\n" +
          " 1 0 0 0 0 0 1 0 0 1 1 1 0 0 1 0 0 0 0 0 1\n" +
          " 1 1 1 1 1 1 1 0 1 0 1 0 1 0 1 1 1 1 1 1 1\n" +
          " 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0\n" +
          " 1 0 1 0 1 0 1 0 0 0 1 0 1 0 0 0 1 0 0 1 0\n" +
          " 1 1 0 1 0 0 0 1 0 1 1 1 0 1 0 1 0 1 0 0 0\n" +
          " 0 1 0 0 0 0 1 1 1 1 1 1 0 1 1 1 0 1 0 1 0\n" +
          " 1 1 1 0 0 1 0 1 0 0 0 1 1 1 0 1 1 0 1 0 0\n" +
          " 0 1 1 0 0 1 1 0 1 1 0 1 0 1 1 1 0 1 0 0 1\n" +
          " 0 0 0 0 0 0 0 0 1 0 1 0 0 0 1 0 0 0 1 0 1\n" +
          " 1 1 1 1 1 1 1 0 0 0 0 0 1 0 0 0 1 0 0 1 1\n" +
          " 1 0 0 0 0 0 1 0 0 0 1 0 0 0 1 0 0 0 1 1 1\n" +
          " 1 0 1 1 1 0 1 0 1 0 0 0 1 0 1 0 1 0 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 0 0 1 0 1 0 1 0 1 0 1 0\n" +
          " 1 0 1 1 1 0 1 0 1 0 1 1 0 1 1 1 0 0 1 0 1\n" +
          " 1 0 0 0 0 0 1 0 0 0 0 1 1 1 0 1 1 1 0 1 0\n" +
          " 1 1 1 1 1 1 1 0 1 1 0 1 0 1 1 1 0 0 1 0 0\n" +
          ">>\n";
    assertEquals(expected, qrCode.toString());
  }

  @Test
  public void testEncodeShiftjisNumeric() throws WriterException {
    Map<EncodeHintType,Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.CHARACTER_SET, "Shift_JIS");
    QRCode qrCode = Encoder.encode("0123", ErrorCorrectionLevel.M, hints);
    String expected =
      "<<\n" +
          " mode: NUMERIC\n" +
          " ecLevel: M\n" +
          " version: 1\n" +
          " maskPattern: 2\n" +
          " matrix:\n" +
          " 1 1 1 1 1 1 1 0 0 1 1 0 1 0 1 1 1 1 1 1 1\n" +
          " 1 0 0 0 0 0 1 0 0 1 0 0 1 0 1 0 0 0 0 0 1\n" +
          " 1 0 1 1 1 0 1 0 1 0 0 0 0 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 1 0 1 1 1 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 1 1 0 1 1 0 1 0 1 1 1 0 1\n" +
          " 1 0 0 0 0 0 1 0 1 1 0 0 1 0 1 0 0 0 0 0 1\n" +
          " 1 1 1 1 1 1 1 0 1 0 1 0 1 0 1 1 1 1 1 1 1\n" +
          " 0 0 0 0 0 0 0 0 1 1 1 1 1 0 0 0 0 0 0 0 0\n" +
          " 1 0 1 1 1 1 1 0 0 1 1 0 1 0 1 1 1 1 1 0 0\n" +
          " 1 1 0 0 0 1 0 0 1 0 1 0 1 0 0 1 0 0 1 0 0\n" +
          " 0 1 1 0 1 1 1 1 0 1 1 1 0 1 0 0 1 1 0 1 1\n" +
          " 1 0 1 1 0 1 0 1 0 0 1 0 0 0 0 1 1 0 1 0 0\n" +
          " 0 0 1 0 0 1 1 1 0 0 0 1 0 1 0 0 1 0 1 0 0\n" +
          " 0 0 0 0 0 0 0 0 1 1 0 1 1 1 1 0 0 1 0 0 0\n" +
          " 1 1 1 1 1 1 1 0 0 0 1 0 1 0 1 1 0 0 0 0 0\n" +
          " 1 0 0 0 0 0 1 0 1 1 0 1 1 1 1 0 0 1 0 1 0\n" +
          " 1 0 1 1 1 0 1 0 1 0 1 0 1 0 0 1 0 0 1 0 0\n" +
          " 1 0 1 1 1 0 1 0 1 1 1 0 1 0 0 1 0 0 1 0 0\n" +
          " 1 0 1 1 1 0 1 0 1 1 0 1 0 1 0 0 1 1 1 0 0\n" +
          " 1 0 0 0 0 0 1 0 0 0 1 0 0 0 0 1 1 0 1 1 0\n" +
          " 1 1 1 1 1 1 1 0 1 1 0 1 0 1 0 0 1 1 1 0 0\n" +
          ">>\n";
    assertEquals(expected, qrCode.toString());
  }

  @Test
  public void testEncodeGS1WithStringTypeHint() throws WriterException {
    Map<EncodeHintType, Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.GS1_FORMAT, "true");
    QRCode qrCode = Encoder.encode("100001%11171218", ErrorCorrectionLevel.H, hints);
    verifyGS1EncodedData(qrCode);
  }

  @Test
  public void testEncodeGS1WithBooleanTypeHint() throws WriterException {
    Map<EncodeHintType, Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.GS1_FORMAT, true);
    QRCode qrCode = Encoder.encode("100001%11171218", ErrorCorrectionLevel.H, hints);
    verifyGS1EncodedData(qrCode);
  }

  @Test
  public void testDoesNotEncodeGS1WhenBooleanTypeHintExplicitlyFalse() throws WriterException {
    Map<EncodeHintType, Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.GS1_FORMAT, false);
    QRCode qrCode = Encoder.encode("ABCDEF", ErrorCorrectionLevel.H, hints);
    verifyNotGS1EncodedData(qrCode);
  }

  @Test
  public void testDoesNotEncodeGS1WhenStringTypeHintExplicitlyFalse() throws WriterException {
    Map<EncodeHintType, Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.GS1_FORMAT, "false");
    QRCode qrCode = Encoder.encode("ABCDEF", ErrorCorrectionLevel.H, hints);
    verifyNotGS1EncodedData(qrCode);
  }

  @Test
  public void testGS1ModeHeaderWithECI() throws WriterException {
    Map<EncodeHintType,Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.CHARACTER_SET, "UTF8");
    hints.put(EncodeHintType.GS1_FORMAT, true);
    QRCode qrCode = Encoder.encode("hello", ErrorCorrectionLevel.H, hints);
    String expected =
      "<<\n" +
          " mode: BYTE\n" +
          " ecLevel: H\n" +
          " version: 1\n" +
          " maskPattern: 5\n" +
          " matrix:\n" +
          " 1 1 1 1 1 1 1 0 1 0 1 1 0 0 1 1 1 1 1 1 1\n" +
          " 1 0 0 0 0 0 1 0 0 1 1 0 0 0 1 0 0 0 0 0 1\n" +
          " 1 0 1 1 1 0 1 0 1 1 1 0 0 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 1 0 1 0 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 1 0 1 0 0 0 1 0 1 1 1 0 1\n" +
          " 1 0 0 0 0 0 1 0 0 1 1 1 1 0 1 0 0 0 0 0 1\n" +
          " 1 1 1 1 1 1 1 0 1 0 1 0 1 0 1 1 1 1 1 1 1\n" +
          " 0 0 0 0 0 0 0 0 1 0 1 1 1 0 0 0 0 0 0 0 0\n" +
          " 0 0 0 0 0 1 1 0 0 1 1 0 0 0 1 0 1 0 1 0 1\n" +
          " 0 1 0 1 1 0 0 1 0 1 1 1 1 1 1 0 1 1 1 0 1\n" +
          " 0 1 0 1 1 1 1 0 1 1 0 0 0 1 0 1 0 1 1 0 0\n" +
          " 1 1 1 1 0 1 0 1 0 0 1 0 1 0 0 1 1 1 1 0 0\n" +
          " 1 0 0 1 0 0 1 1 0 1 1 0 1 0 1 0 0 1 0 0 1\n" +
          " 0 0 0 0 0 0 0 0 1 1 1 1 1 0 1 0 1 0 0 1 0\n" +
          " 1 1 1 1 1 1 1 0 0 0 1 1 0 0 1 0 0 0 1 1 0\n" +
          " 1 0 0 0 0 0 1 0 1 1 0 0 0 0 1 0 1 1 1 0 0\n" +
          " 1 0 1 1 1 0 1 0 0 1 0 0 1 0 1 0 1 0 0 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 0 0 0 1 1 1 0 1 1 1 1 0\n" +
          " 1 0 1 1 1 0 1 0 0 0 1 0 0 1 0 0 1 0 1 1 1\n" +
          " 1 0 0 0 0 0 1 0 0 1 0 0 0 1 1 0 0 1 1 1 1\n" +
          " 1 1 1 1 1 1 1 0 0 1 1 1 0 1 1 0 1 0 0 1 0\n" +
          ">>\n";
    assertEquals(expected, qrCode.toString());
  }

  @Test
  public void testAppendModeInfo() {
    BitArray bits = new BitArray();
    Encoder.appendModeInfo(Mode.NUMERIC, bits);
    assertEquals(" ...X", bits.toString());
  }

  @Test
  public void testAppendLengthInfo() throws WriterException {
    BitArray bits = new BitArray();
    Encoder.appendLengthInfo(1,                               Version.getVersionForNumber(1),
                             Mode.NUMERIC,
                             bits);
    assertEquals(" ........ .X", bits.toString());      bits = new BitArray();
    Encoder.appendLengthInfo(2,                               Version.getVersionForNumber(10),
                             Mode.ALPHANUMERIC,
                             bits);
    assertEquals(" ........ .X.", bits.toString());      bits = new BitArray();
    Encoder.appendLengthInfo(255,                               Version.getVersionForNumber(27),
                             Mode.BYTE,
                             bits);
    assertEquals(" ........ XXXXXXXX", bits.toString());      bits = new BitArray();
    Encoder.appendLengthInfo(512,                               Version.getVersionForNumber(40),
                             Mode.KANJI,
                             bits);
    assertEquals(" ..X..... ....", bits.toString());    }

  @Test
  public void testAppendBytes() throws WriterException {
            BitArray bits = new BitArray();
    Encoder.appendBytes("1", Mode.NUMERIC, bits, Encoder.DEFAULT_BYTE_MODE_ENCODING);
    assertEquals(" ...X" , bits.toString());
            bits = new BitArray();
    Encoder.appendBytes("A", Mode.ALPHANUMERIC, bits, Encoder.DEFAULT_BYTE_MODE_ENCODING);
    assertEquals(" ..X.X." , bits.toString());
        try {
      Encoder.appendBytes("a", Mode.ALPHANUMERIC, bits, Encoder.DEFAULT_BYTE_MODE_ENCODING);
    } catch (WriterException we) {
          }
            bits = new BitArray();
    Encoder.appendBytes("abc", Mode.BYTE, bits, Encoder.DEFAULT_BYTE_MODE_ENCODING);
    assertEquals(" .XX....X .XX...X. .XX...XX", bits.toString());
        Encoder.appendBytes("\0", Mode.BYTE, bits, Encoder.DEFAULT_BYTE_MODE_ENCODING);
            bits = new BitArray();
    Encoder.appendBytes(shiftJISString(new byte[] {(byte)0x93,0x5f}), Mode.KANJI, bits, Encoder.DEFAULT_BYTE_MODE_ENCODING);
    assertEquals(" .XX.XX.. XXXXX", bits.toString());
  }

  @Test
  public void testTerminateBits() throws WriterException {
    BitArray v = new BitArray();
    Encoder.terminateBits(0, v);
    assertEquals("", v.toString());
    v = new BitArray();
    Encoder.terminateBits(1, v);
    assertEquals(" ........", v.toString());
    v = new BitArray();
    v.appendBits(0, 3);      Encoder.terminateBits(1, v);
    assertEquals(" ........", v.toString());
    v = new BitArray();
    v.appendBits(0, 5);      Encoder.terminateBits(1, v);
    assertEquals(" ........", v.toString());
    v = new BitArray();
    v.appendBits(0, 8);      Encoder.terminateBits(1, v);
    assertEquals(" ........", v.toString());
    v = new BitArray();
    Encoder.terminateBits(2, v);
    assertEquals(" ........ XXX.XX..", v.toString());
    v = new BitArray();
    v.appendBits(0, 1);      Encoder.terminateBits(3, v);
    assertEquals(" ........ XXX.XX.. ...X...X", v.toString());
  }

  @Test
  public void testGetNumDataBytesAndNumECBytesForBlockID() throws WriterException {
    int[] numDataBytes = new int[1];
    int[] numEcBytes = new int[1];
        Encoder.getNumDataBytesAndNumECBytesForBlockID(26, 9, 1, 0, numDataBytes, numEcBytes);
    assertEquals(9, numDataBytes[0]);
    assertEquals(17, numEcBytes[0]);

        Encoder.getNumDataBytesAndNumECBytesForBlockID(70, 26, 2, 0, numDataBytes, numEcBytes);
    assertEquals(13, numDataBytes[0]);
    assertEquals(22, numEcBytes[0]);
    Encoder.getNumDataBytesAndNumECBytesForBlockID(70, 26, 2, 1, numDataBytes, numEcBytes);
    assertEquals(13, numDataBytes[0]);
    assertEquals(22, numEcBytes[0]);

        Encoder.getNumDataBytesAndNumECBytesForBlockID(196, 66, 5, 0, numDataBytes, numEcBytes);
    assertEquals(13, numDataBytes[0]);
    assertEquals(26, numEcBytes[0]);
    Encoder.getNumDataBytesAndNumECBytesForBlockID(196, 66, 5, 4, numDataBytes, numEcBytes);
    assertEquals(14, numDataBytes[0]);
    assertEquals(26, numEcBytes[0]);

        Encoder.getNumDataBytesAndNumECBytesForBlockID(3706, 1276, 81, 0, numDataBytes, numEcBytes);
    assertEquals(15, numDataBytes[0]);
    assertEquals(30, numEcBytes[0]);
    Encoder.getNumDataBytesAndNumECBytesForBlockID(3706, 1276, 81, 20, numDataBytes, numEcBytes);
    assertEquals(16, numDataBytes[0]);
    assertEquals(30, numEcBytes[0]);
    Encoder.getNumDataBytesAndNumECBytesForBlockID(3706, 1276, 81, 80, numDataBytes, numEcBytes);
    assertEquals(16, numDataBytes[0]);
    assertEquals(30, numEcBytes[0]);
  }

  @Test
  public void testInterleaveWithECBytes() throws WriterException {
    byte[] dataBytes = {32, 65, (byte)205, 69, 41, (byte)220, 46, (byte)128, (byte)236};
    BitArray in = new BitArray();
    for (byte dataByte: dataBytes) {
      in.appendBits(dataByte, 8);
    }
    BitArray out = Encoder.interleaveWithECBytes(in, 26, 9, 1);
    byte[] expected = {
                32, 65, (byte)205, 69, 41, (byte)220, 46, (byte)128, (byte)236,
                42, (byte)159, 74, (byte)221, (byte)244, (byte)169, (byte)239, (byte)150, (byte)138, 70,
        (byte)237, 85, (byte)224, 96, 74, (byte)219, 61,
    };
    assertEquals(expected.length, out.getSizeInBytes());
    byte[] outArray = new byte[expected.length];
    out.toBytes(0, outArray, 0, expected.length);
        for (int x = 0; x < expected.length; x++) {
      assertEquals(expected[x], outArray[x]);
    }
        dataBytes = new byte[] {
        67, 70, 22, 38, 54, 70, 86, 102, 118, (byte)134, (byte)150, (byte)166, (byte)182,
        (byte)198, (byte)214, (byte)230, (byte)247, 7, 23, 39, 55, 71, 87, 103, 119, (byte)135,
        (byte)151, (byte)166, 22, 38, 54, 70, 86, 102, 118, (byte)134, (byte)150, (byte)166,
        (byte)182, (byte)198, (byte)214, (byte)230, (byte)247, 7, 23, 39, 55, 71, 87, 103, 119,
        (byte)135, (byte)151, (byte)160, (byte)236, 17, (byte)236, 17, (byte)236, 17, (byte)236,
        17
    };
    in = new BitArray();
    for (byte dataByte: dataBytes) {
      in.appendBits(dataByte, 8);
    }

    out = Encoder.interleaveWithECBytes(in, 134, 62, 4);
    expected = new byte[] {
                67, (byte)230, 54, 55, 70, (byte)247, 70, 71, 22, 7, 86, 87, 38, 23, 102, 103, 54, 39,
        118, 119, 70, 55, (byte)134, (byte)135, 86, 71, (byte)150, (byte)151, 102, 87, (byte)166,
        (byte)160, 118, 103, (byte)182, (byte)236, (byte)134, 119, (byte)198, 17, (byte)150,
        (byte)135, (byte)214, (byte)236, (byte)166, (byte)151, (byte)230, 17, (byte)182,
        (byte)166, (byte)247, (byte)236, (byte)198, 22, 7, 17, (byte)214, 38, 23, (byte)236, 39,
        17,
                (byte)175, (byte)155, (byte)245, (byte)236, 80, (byte)146, 56, 74, (byte)155, (byte)165,
        (byte)133, (byte)142, 64, (byte)183, (byte)132, 13, (byte)178, 54, (byte)132, 108, 45,
        113, 53, 50, (byte)214, 98, (byte)193, (byte)152, (byte)233, (byte)147, 50, 71, 65,
        (byte)190, 82, 51, (byte)209, (byte)199, (byte)171, 54, 12, 112, 57, 113, (byte)155, 117,
        (byte)211, (byte)164, 117, 30, (byte)158, (byte)225, 31, (byte)190, (byte)242, 38,
        (byte)140, 61, (byte)179, (byte)154, (byte)214, (byte)138, (byte)147, 87, 27, 96, 77, 47,
        (byte)187, 49, (byte)156, (byte)214,
    };
    assertEquals(expected.length, out.getSizeInBytes());
    outArray = new byte[expected.length];
    out.toBytes(0, outArray, 0, expected.length);
    for (int x = 0; x < expected.length; x++) {
      assertEquals(expected[x], outArray[x]);
    }
  }

  @Test
  public void testAppendNumericBytes() {
        BitArray bits = new BitArray();
    Encoder.appendNumericBytes("1", bits);
    assertEquals(" ...X" , bits.toString());
        bits = new BitArray();
    Encoder.appendNumericBytes("12", bits);
    assertEquals(" ...XX.." , bits.toString());
        bits = new BitArray();
    Encoder.appendNumericBytes("123", bits);
    assertEquals(" ...XXXX. XX" , bits.toString());
        bits = new BitArray();
    Encoder.appendNumericBytes("1234", bits);
    assertEquals(" ...XXXX. XX.X.." , bits.toString());
        bits = new BitArray();
    Encoder.appendNumericBytes("", bits);
    assertEquals("" , bits.toString());
  }

  @Test
  public void testAppendAlphanumericBytes() throws WriterException {
        BitArray bits = new BitArray();
    Encoder.appendAlphanumericBytes("A", bits);
    assertEquals(" ..X.X." , bits.toString());
        bits = new BitArray();
    Encoder.appendAlphanumericBytes("AB", bits);
    assertEquals(" ..XXX..X X.X", bits.toString());
        bits = new BitArray();
    Encoder.appendAlphanumericBytes("ABC", bits);
    assertEquals(" ..XXX..X X.X..XX. ." , bits.toString());
        bits = new BitArray();
    Encoder.appendAlphanumericBytes("", bits);
    assertEquals("" , bits.toString());
        try {
      Encoder.appendAlphanumericBytes("abc", new BitArray());
    } catch (WriterException we) {
          }
  }

  @Test
  public void testAppend8BitBytes() throws WriterException {
        BitArray bits = new BitArray();
    Encoder.append8BitBytes("abc", bits, Encoder.DEFAULT_BYTE_MODE_ENCODING);
    assertEquals(" .XX....X .XX...X. .XX...XX", bits.toString());
        bits = new BitArray();
    Encoder.append8BitBytes("", bits, Encoder.DEFAULT_BYTE_MODE_ENCODING);
    assertEquals("", bits.toString());
  }

    @Test
  public void testAppendKanjiBytes() throws WriterException {
    BitArray bits = new BitArray();
      Encoder.appendKanjiBytes(shiftJISString(new byte[] {(byte)0x93,0x5f}), bits);
      assertEquals(" .XX.XX.. XXXXX", bits.toString());
      Encoder.appendKanjiBytes(shiftJISString(new byte[] {(byte)0xe4,(byte)0xaa}), bits);
      assertEquals(" .XX.XX.. XXXXXXX. X.X.X.X. X.", bits.toString());
  }

      @Test
  public void testGenerateECBytes() {
    byte[] dataBytes = {32, 65, (byte)205, 69, 41, (byte)220, 46, (byte)128, (byte)236};
    byte[] ecBytes = Encoder.generateECBytes(dataBytes, 17);
    int[] expected = {
        42, 159, 74, 221, 244, 169, 239, 150, 138, 70, 237, 85, 224, 96, 74, 219, 61
    };
    assertEquals(expected.length, ecBytes.length);
    for (int x = 0; x < expected.length; x++) {
      assertEquals(expected[x], ecBytes[x] & 0xFF);
    }
    dataBytes = new byte[] {67, 70, 22, 38, 54, 70, 86, 102, 118,
        (byte)134, (byte)150, (byte)166, (byte)182, (byte)198, (byte)214};
    ecBytes = Encoder.generateECBytes(dataBytes, 18);
    expected = new int[] {
        175, 80, 155, 64, 178, 45, 214, 233, 65, 209, 12, 155, 117, 31, 140, 214, 27, 187
    };
    assertEquals(expected.length, ecBytes.length);
    for (int x = 0; x < expected.length; x++) {
      assertEquals(expected[x], ecBytes[x] & 0xFF);
    }
        dataBytes = new byte[] {32, 49, (byte)205, 69, 42, 20, 0, (byte)236, 17};
    ecBytes = Encoder.generateECBytes(dataBytes, 17);
    expected = new int[] {
        0, 3, 130, 179, 194, 0, 55, 211, 110, 79, 98, 72, 170, 96, 211, 137, 213
    };
    assertEquals(expected.length, ecBytes.length);
    for (int x = 0; x < expected.length; x++) {
      assertEquals(expected[x], ecBytes[x] & 0xFF);
    }
  }

  @Test
  public void testBugInBitVectorNumBytes() throws WriterException {
                                                                                                                    StringBuilder builder = new StringBuilder(3518);
    for (int x = 0; x < 3518; x++) {
      builder.append('0');
    }
    Encoder.encode(builder.toString(), ErrorCorrectionLevel.L);
  }

  private void verifyGS1EncodedData(QRCode qrCode) {
    String expected =
      "<<\n" +
          " mode: ALPHANUMERIC\n" +
          " ecLevel: H\n" +
          " version: 2\n" +
          " maskPattern: 4\n" +
          " matrix:\n" +
          " 1 1 1 1 1 1 1 0 0 1 1 1 1 0 1 0 1 0 1 1 1 1 1 1 1\n" +
          " 1 0 0 0 0 0 1 0 1 1 0 0 0 0 0 1 1 0 1 0 0 0 0 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 0 0 0 1 1 1 0 1 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 1 0 1 0 0 1 1 0 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 0 1 1 1 0 0 0 1 0 1 0 1 1 1 0 1\n" +
          " 1 0 0 0 0 0 1 0 1 1 0 1 1 0 1 1 0 0 1 0 0 0 0 0 1\n" +
          " 1 1 1 1 1 1 1 0 1 0 1 0 1 0 1 0 1 0 1 1 1 1 1 1 1\n" +
          " 0 0 0 0 0 0 0 0 1 1 0 1 1 0 1 1 0 0 0 0 0 0 0 0 0\n" +
          " 0 0 0 0 1 1 1 1 0 0 1 1 0 0 0 1 1 0 1 1 0 0 0 1 0\n" +
          " 0 1 1 0 1 1 0 0 1 1 1 0 0 0 1 1 1 1 1 1 1 0 0 0 1\n" +
          " 0 0 1 1 1 1 1 0 1 1 1 1 1 0 1 0 0 0 0 0 0 1 1 1 0\n" +
          " 1 0 1 1 1 0 0 1 1 1 0 1 1 1 1 1 0 1 1 0 1 1 1 0 0\n" +
          " 0 1 0 1 0 0 1 1 1 1 1 1 0 0 1 1 0 1 0 0 0 0 0 1 0\n" +
          " 1 0 0 1 1 1 0 0 1 1 0 0 0 1 1 0 1 0 1 0 1 0 0 0 0\n" +
          " 0 0 1 0 0 1 1 1 0 1 1 0 1 1 1 0 1 1 1 0 1 1 1 1 0\n" +
          " 0 0 0 1 1 0 0 1 0 0 1 0 0 1 1 0 0 1 0 0 0 1 1 1 0\n" +
          " 1 1 0 1 0 1 1 0 1 0 1 0 0 0 1 1 1 1 1 1 1 0 0 0 0\n" +
          " 0 0 0 0 0 0 0 0 1 1 0 1 0 0 0 1 1 0 0 0 1 1 0 1 0\n" +
          " 1 1 1 1 1 1 1 0 1 0 1 0 1 0 1 1 1 0 1 0 1 0 0 0 0\n" +
          " 1 0 0 0 0 0 1 0 1 1 0 0 0 1 0 1 1 0 0 0 1 0 1 1 0\n" +
          " 1 0 1 1 1 0 1 0 1 1 1 0 0 0 0 0 1 1 1 1 1 1 0 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 0 0 0 0 1 1 1 0 0 1 1 0 1 0 0 0\n" +
          " 1 0 1 1 1 0 1 0 0 0 1 1 0 1 0 1 1 1 0 1 1 0 0 1 0\n" +
          " 1 0 0 0 0 0 1 0 0 1 1 0 1 1 1 1 1 0 1 0 1 1 0 0 0\n" +
          " 1 1 1 1 1 1 1 0 0 0 1 0 0 0 0 1 1 0 0 1 1 0 0 1 1\n" +
          ">>\n";
    assertEquals(expected, qrCode.toString());
  }

  private void verifyNotGS1EncodedData(QRCode qrCode) {
    String expected =
      "<<\n" +
          " mode: ALPHANUMERIC\n" +
          " ecLevel: H\n" +
          " version: 1\n" +
          " maskPattern: 4\n" +
          " matrix:\n" +
          " 1 1 1 1 1 1 1 0 0 1 0 1 0 0 1 1 1 1 1 1 1\n" +
          " 1 0 0 0 0 0 1 0 1 0 1 0 1 0 1 0 0 0 0 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 0 0 0 0 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 1 0 0 1 0 1 0 1 1 1 0 1\n" +
          " 1 0 1 1 1 0 1 0 0 1 0 1 0 0 1 0 1 1 1 0 1\n" +
          " 1 0 0 0 0 0 1 0 1 0 0 1 1 0 1 0 0 0 0 0 1\n" +
          " 1 1 1 1 1 1 1 0 1 0 1 0 1 0 1 1 1 1 1 1 1\n" +
          " 0 0 0 0 0 0 0 0 1 0 0 0 1 0 0 0 0 0 0 0 0\n" +
          " 0 0 0 0 1 1 1 1 0 1 1 0 1 0 1 1 0 0 0 1 0\n" +
          " 0 0 0 0 1 1 0 1 1 1 0 0 1 1 1 1 0 1 1 0 1\n" +
          " 1 0 0 0 0 1 1 0 0 1 0 1 0 0 0 1 1 1 0 1 1\n" +
          " 1 0 0 1 1 1 0 0 1 1 1 1 0 0 0 0 1 0 0 0 0\n" +
          " 0 1 1 1 1 1 1 0 1 0 1 0 1 1 1 0 0 1 1 0 0\n" +
          " 0 0 0 0 0 0 0 0 1 1 0 0 0 1 1 0 0 0 1 0 1\n" +
          " 1 1 1 1 1 1 1 0 1 1 1 1 0 0 0 0 0 1 1 0 0\n" +
          " 1 0 0 0 0 0 1 0 1 1 0 1 0 0 0 1 0 1 1 1 1\n" +
          " 1 0 1 1 1 0 1 0 1 0 0 1 0 0 0 1 1 0 0 1 1\n" +
          " 1 0 1 1 1 0 1 0 0 0 1 1 0 1 0 0 0 0 1 1 1\n" +
          " 1 0 1 1 1 0 1 0 0 1 0 1 0 0 0 1 1 0 0 0 0\n" +
          " 1 0 0 0 0 0 1 0 0 1 0 0 1 0 0 1 1 0 0 0 1\n" +
          " 1 1 1 1 1 1 1 0 0 0 1 0 0 1 0 0 0 0 1 1 1\n" +
          ">>\n";
    assertEquals(expected, qrCode.toString());
  }

  private static String shiftJISString(byte[] bytes) throws WriterException {
    try {
      return new String(bytes, "Shift_JIS");
    } catch (UnsupportedEncodingException uee) {
      throw new WriterException(uee.toString());
    }
  }

}
