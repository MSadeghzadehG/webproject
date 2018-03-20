

package com.google.zxing.datamatrix.encoder;

import java.nio.charset.StandardCharsets;

import junit.framework.ComparisonFailure;
import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;


public final class HighLevelEncodeTestCase extends Assert {

  private static final SymbolInfo[] TEST_SYMBOLS = {
    new SymbolInfo(false, 3, 5, 8, 8, 1),
    new SymbolInfo(false, 5, 7, 10, 10, 1),
      new SymbolInfo(true, 5, 7, 16, 6, 1),
    new SymbolInfo(false, 8, 10, 12, 12, 1),
      new SymbolInfo(true, 10, 11, 14, 6, 2),
    new SymbolInfo(false, 13, 0, 0, 0, 1),
    new SymbolInfo(false, 77, 0, 0, 0, 1)
      };

  private static void useTestSymbols() {
    SymbolInfo.overrideSymbolSet(TEST_SYMBOLS);
  }

  private static void resetSymbols() {
    SymbolInfo.overrideSymbolSet(SymbolInfo.PROD_SYMBOLS);
  }

  @Test
  public void testASCIIEncodation() {

    String visualized = encodeHighLevel("123456");
    assertEquals("142 164 186", visualized);

    visualized = encodeHighLevel("123456£");
    assertEquals("142 164 186 235 36", visualized);

    visualized = encodeHighLevel("30Q324343430794<OQQ");
    assertEquals("160 82 162 173 173 173 137 224 61 80 82 82", visualized);
  }

  @Test
  public void testC40EncodationBasic1() {

    String visualized = encodeHighLevel("AIMAIMAIM");
    assertEquals("230 91 11 91 11 91 11 254", visualized);
      }

  @Test  
  public void testC40EncodationBasic2() {

    String visualized = encodeHighLevel("AIMAIAB");
    assertEquals("230 91 11 90 255 254 67 129", visualized);
        
    visualized = encodeHighLevel("AIMAIAb");
    assertEquals("66 74 78 66 74 66 99 129", visualized);                 
    visualized = encodeHighLevel("AIMAIMAIMË");
    assertEquals("230 91 11 91 11 91 11 254 235 76", visualized);
                    
    visualized = encodeHighLevel("AIMAIMAIMë");
    assertEquals("230 91 11 91 11 91 11 254 235 108", visualized);                       }

  @Test  
  public void testC40EncodationSpecExample() {
        String visualized = encodeHighLevel("A1B2C3D4E5F6G7H8I9J0K1L2");
    assertEquals("230 88 88 40 8 107 147 59 67 126 206 78 126 144 121 35 47 254", visualized);
  }

  @Test  
  public void testC40EncodationSpecialCases1() {

            useTestSymbols();

    String visualized = encodeHighLevel("AIMAIMAIMAIMAIMAIM");
    assertEquals("230 91 11 91 11 91 11 91 11 91 11 91 11", visualized);
    
    visualized = encodeHighLevel("AIMAIMAIMAIMAIMAI");
    assertEquals("230 91 11 91 11 91 11 91 11 91 11 90 241", visualized);
    
    visualized = encodeHighLevel("AIMAIMAIMAIMAIMA");
    assertEquals("230 91 11 91 11 91 11 91 11 91 11 254 66", visualized);
    
    resetSymbols();

    visualized = encodeHighLevel("AIMAIMAIMAIMAIMAI");
    assertEquals("230 91 11 91 11 91 11 91 11 91 11 254 66 74 129 237", visualized);

    visualized = encodeHighLevel("AIMAIMAIMA");
    assertEquals("230 91 11 91 11 91 11 66", visualized);
      }

  @Test  
  public void testC40EncodationSpecialCases2() {

    String visualized = encodeHighLevel("AIMAIMAIMAIMAIMAIMAI");
    assertEquals("230 91 11 91 11 91 11 91 11 91 11 91 11 254 66 74", visualized);
      }

  @Test  
  public void testTextEncodation() {

    String visualized = encodeHighLevel("aimaimaim");
    assertEquals("239 91 11 91 11 91 11 254", visualized);
    
    visualized = encodeHighLevel("aimaimaim'");
    assertEquals("239 91 11 91 11 91 11 254 40 129", visualized);
        
    visualized = encodeHighLevel("aimaimaIm");
    assertEquals("239 91 11 91 11 87 218 110", visualized);

    visualized = encodeHighLevel("aimaimaimB");
    assertEquals("239 91 11 91 11 91 11 254 67 129", visualized);

    visualized = encodeHighLevel("aimaimaim{txt}\u0004");
    assertEquals("239 91 11 91 11 91 11 16 218 236 107 181 69 254 129 237", visualized);
  }

  @Test  
  public void testX12Encodation() {

    
    String visualized = encodeHighLevel("ABC>ABC123>AB");
    assertEquals("238 89 233 14 192 100 207 44 31 67", visualized);

    visualized = encodeHighLevel("ABC>ABC123>ABC");
    assertEquals("238 89 233 14 192 100 207 44 31 254 67 68", visualized);

    visualized = encodeHighLevel("ABC>ABC123>ABCD");
    assertEquals("238 89 233 14 192 100 207 44 31 96 82 254", visualized);

    visualized = encodeHighLevel("ABC>ABC123>ABCDE");
    assertEquals("238 89 233 14 192 100 207 44 31 96 82 70", visualized);

    visualized = encodeHighLevel("ABC>ABC123>ABCDEF");
    assertEquals("238 89 233 14 192 100 207 44 31 96 82 254 70 71 129 237", visualized);

  }

  @Test  
  public void testEDIFACTEncodation() {

    
    String visualized = encodeHighLevel(".A.C1.3.DATA.123DATA.123DATA");
    assertEquals("240 184 27 131 198 236 238 16 21 1 187 28 179 16 21 1 187 28 179 16 21 1",
                 visualized);

    visualized = encodeHighLevel(".A.C1.3.X.X2..");
    assertEquals("240 184 27 131 198 236 238 98 230 50 47 47", visualized);

    visualized = encodeHighLevel(".A.C1.3.X.X2.");
    assertEquals("240 184 27 131 198 236 238 98 230 50 47 129", visualized);

    visualized = encodeHighLevel(".A.C1.3.X.X2");
    assertEquals("240 184 27 131 198 236 238 98 230 50", visualized);

    visualized = encodeHighLevel(".A.C1.3.X.X");
    assertEquals("240 184 27 131 198 236 238 98 230 31", visualized);

    visualized = encodeHighLevel(".A.C1.3.X.");
    assertEquals("240 184 27 131 198 236 238 98 231 192", visualized);

    visualized = encodeHighLevel(".A.C1.3.X");
    assertEquals("240 184 27 131 198 236 238 89", visualized);

        visualized = encodeHighLevel(".XXX.XXX.XXX.XXX.XXX.XXX.üXX.XXX.XXX.XXX.XXX.XXX.XXX");
    assertEquals("240 185 134 24 185 134 24 185 134 24 185 134 24 185 134 24 185 134 24"
                     + " 124 47 235 125 240"                      + " 97 139 152 97 139 152 97 139 152 97 139 152 97 139 152 97 139 152 89 89",
                 visualized);
  }

  @Test  
  public void testBase256Encodation() {

    
    String visualized = encodeHighLevel("\u00ABäöüé\u00BB");
    assertEquals("231 44 108 59 226 126 1 104", visualized);
    visualized = encodeHighLevel("\u00ABäöüéà\u00BB");
    assertEquals("231 51 108 59 226 126 1 141 254 129", visualized);
    visualized = encodeHighLevel("\u00ABäöüéàá\u00BB");
    assertEquals("231 44 108 59 226 126 1 141 36 147", visualized);

    visualized = encodeHighLevel(" 23£");     assertEquals("33 153 235 36 129", visualized);

    visualized = encodeHighLevel("\u00ABäöüé\u00BB 234");     assertEquals("231 51 108 59 226 126 1 104 99 153 53 129", visualized);

    visualized = encodeHighLevel("\u00ABäöüé\u00BB 23£ 1234567890123456789");
    assertEquals("231 55 108 59 226 126 1 104 99 10 161 167 185 142 164 186 208"
                     + " 220 142 164 186 208 58 129 59 209 104 254 150 45", visualized);

    visualized = encodeHighLevel(createBinaryMessage(20));
    assertEquals("231 44 108 59 226 126 1 141 36 5 37 187 80 230 123 17 166 60 210 103 253 150",
                 visualized);
    visualized = encodeHighLevel(createBinaryMessage(19));     assertEquals("231 63 108 59 226 126 1 141 36 5 37 187 80 230 123 17 166 60 210 103 1 129",
                 visualized);

    visualized = encodeHighLevel(createBinaryMessage(276));
    assertStartsWith("231 38 219 2 208 120 20 150 35", visualized);
    assertEndsWith("146 40 194 129", visualized);

    visualized = encodeHighLevel(createBinaryMessage(277));
    assertStartsWith("231 38 220 2 208 120 20 150 35", visualized);
    assertEndsWith("146 40 190 87", visualized);
  }

  private static String createBinaryMessage(int len) {
    StringBuilder sb = new StringBuilder();
    sb.append("\u00ABäöüéàá-");
    for (int i = 0; i < len - 9; i++) {
      sb.append('\u00B7');
    }
    sb.append('\u00BB');
    return sb.toString();
  }

  private static void assertStartsWith(String expected, String actual) {
    if (!actual.startsWith(expected)) {
      throw new ComparisonFailure(null, expected, actual.substring(0, expected.length()));
    }
  }

  private static void assertEndsWith(String expected, String actual) {
    if (!actual.endsWith(expected)) {
      throw new ComparisonFailure(null, expected, actual.substring(actual.length() - expected.length()));
    }
  }

  @Test  
  public void testUnlatchingFromC40() {

    String visualized = encodeHighLevel("AIMAIMAIMAIMaimaimaim");
    assertEquals("230 91 11 91 11 91 11 254 66 74 78 239 91 11 91 11 91 11", visualized);
  }

  @Test  
  public void testUnlatchingFromText() {

    String visualized = encodeHighLevel("aimaimaimaim12345678");
    assertEquals("239 91 11 91 11 91 11 91 11 254 142 164 186 208 129 237", visualized);
  }

  @Test  
  public void testHelloWorld() {

    String visualized = encodeHighLevel("Hello World!");
    assertEquals("73 239 116 130 175 123 148 64 158 233 254 34", visualized);
  }

  @Test  
  public void testBug1664266() {
        
    String visualized = encodeHighLevel("CREX-TAN:h");
    assertEquals("240 13 33 88 181 64 78 124 59 105", visualized);

    visualized = encodeHighLevel("CREX-TAN:hh");
    assertEquals("240 13 33 88 181 64 78 124 59 105 105 129", visualized);

    visualized = encodeHighLevel("CREX-TAN:hhh");
    assertEquals("240 13 33 88 181 64 78 124 59 105 105 105", visualized);
  }

  @Test
  public void testX12Unlatch() {
    String visualized = encodeHighLevel("*DTCP01");
    assertEquals("238 9 10 104 141 254 50 129", visualized);
  }

  @Test
  public void testX12Unlatch2() {
    String visualized = encodeHighLevel("*DTCP0");
    assertEquals("238 9 10 104 141", visualized);
  }

  @Test  
  public void testBug3048549() {
        
    String visualized = encodeHighLevel("fiykmj*Rh2`,e6");
    assertEquals("239 122 87 154 40 7 171 115 207 12 130 71 155 254 129 237", visualized);

  }

  @Test  
  public void testMacroCharacters() {

    String visualized = encodeHighLevel("[)>\u001E05\u001D5555\u001C6666\u001E\u0004");
        assertEquals("236 185 185 29 196 196 129 56", visualized);
  }

  @Test
  public void testEncodingWithStartAsX12AndLatchToEDIFACTInTheMiddle() {

    String visualized = encodeHighLevel("*MEMANT-1F-MESTECH");
    assertEquals("238 10 99 164 204 254 240 82 220 70 180 209 83 80 80 200", visualized);
  }

  @Ignore
  @Test  
  public void testDataURL() {

    byte[] data = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
        0x7E, 0x7F, (byte) 0x80, (byte) 0x81, (byte) 0x82};
    String expected = encodeHighLevel(new String(data, StandardCharsets.ISO_8859_1));
    String visualized = encodeHighLevel("url(data:text/plain;charset=iso-8859-1,"
                                            + "%00%01%02%03%04%05%06%07%08%09%0A%7E%7F%80%81%82)");
    assertEquals(expected, visualized);
    assertEquals("1 2 3 4 5 6 7 8 9 10 11 231 153 173 67 218 112 7", visualized);

    visualized = encodeHighLevel("url(data:;base64,flRlc3R+)");
    assertEquals("127 85 102 116 117 127 129 56", visualized);
  }

  private static String encodeHighLevel(String msg) {
    CharSequence encoded = HighLevelEncoder.encodeHighLevel(msg);
        return visualize(encoded);
  }
  
  
  static String visualize(CharSequence codewords) {
    StringBuilder sb = new StringBuilder();
    for (int i = 0; i < codewords.length(); i++) {
      if (i > 0) {
        sb.append(' ');
      }
      sb.append((int) codewords.charAt(i));
    }
    return sb.toString();
  }

}
