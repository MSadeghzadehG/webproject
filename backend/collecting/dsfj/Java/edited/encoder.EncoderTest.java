

package com.google.zxing.aztec.encoder;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.FormatException;
import com.google.zxing.ResultPoint;
import com.google.zxing.aztec.AztecDetectorResult;
import com.google.zxing.aztec.AztecWriter;
import com.google.zxing.aztec.decoder.Decoder;
import com.google.zxing.common.BitArray;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.common.DecoderResult;
import org.junit.Assert;
import org.junit.Test;

import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.EnumMap;
import java.util.Map;
import java.util.Random;
import java.util.regex.Pattern;


public final class EncoderTest extends Assert {

  private static final Pattern DOTX = Pattern.compile("[^.X]");
  private static final Pattern SPACES = Pattern.compile("\\s+");
  private static final ResultPoint[] NO_POINTS = new ResultPoint[0];

  
  @Test
  public void testEncode1() throws Exception {
    testEncode("This is an example Aztec symbol for Wikipedia.", true, 3,
        "X     X X       X     X X     X     X         \n" +
        "X         X     X X     X   X X   X X       X \n" +
        "X X   X X X X X   X X X                 X     \n" +
        "X X                 X X   X       X X X X X X \n" +
        "    X X X   X   X     X X X X         X X     \n" +
        "  X X X   X X X X   X     X   X     X X   X   \n" +
        "        X X X X X     X X X X   X   X     X   \n" +
        "X       X   X X X X X X X X X X X     X   X X \n" +
        "X   X     X X X               X X X X   X X   \n" +
        "X     X X   X X   X X X X X   X X   X   X X X \n" +
        "X   X         X   X       X   X X X X       X \n" +
        "X       X     X   X   X   X   X   X X   X     \n" +
        "      X   X X X   X       X   X     X X X     \n" +
        "    X X X X X X   X X X X X   X X X X X X   X \n" +
        "  X X   X   X X               X X X   X X X X \n" +
        "  X   X       X X X X X X X X X X X X   X X   \n" +
        "  X X   X       X X X   X X X       X X       \n" +
        "  X               X   X X     X     X X X     \n" +
        "  X   X X X   X X   X   X X X X   X   X X X X \n" +
        "    X   X   X X X   X   X   X X X X     X     \n" +
        "        X               X                 X   \n" +
        "        X X     X   X X   X   X   X       X X \n" +
        "  X   X   X X       X   X         X X X     X \n");
  }
  
  @Test
  public void testEncode2() throws Exception {
    testEncode("Aztec Code is a public domain 2D matrix barcode symbology" +
                " of nominally square symbols built on a square grid with a " +
                "distinctive square bullseye pattern at their center.", false, 6,
          "        X X     X X     X     X     X   X X X         X   X         X   X X       \n" +
          "  X       X X     X   X X   X X       X             X     X   X X   X           X \n" +
          "  X   X X X     X   X   X X     X X X   X   X X               X X       X X     X \n" +
          "X X X             X   X         X         X     X     X   X     X X       X   X   \n" +
          "X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n" +
          "    X X   X   X   X X X               X       X       X X     X X   X X       X   \n" +
          "X X     X       X       X X X X   X   X X       X   X X   X       X X   X X   X   \n" +
          "  X       X   X     X X   X   X X   X X   X X X X X X   X X           X   X   X X \n" +
          "X X   X X   X   X X X X   X X X X X X X X   X   X       X X   X X X X   X X X     \n" +
          "  X       X   X     X       X X     X X   X   X   X     X X   X X X   X     X X X \n" +
          "  X   X X X   X X       X X X         X X           X   X   X   X X X   X X     X \n" +
          "    X     X   X X     X X X X     X   X     X X X X   X X   X X   X X X     X   X \n" +
          "X X X   X             X         X X X X X   X   X X   X   X   X X   X   X   X   X \n" +
          "          X       X X X   X X     X   X           X   X X X X   X X               \n" +
          "  X     X X   X   X       X X X X X X X X X X X X X X X   X   X X   X   X X X     \n" +
          "    X X                 X   X                       X X   X       X         X X X \n" +
          "        X   X X   X X X X X X   X X X X X X X X X   X     X X           X X X X   \n" +
          "          X X X   X     X   X   X               X   X X     X X X   X X           \n" +
          "X X     X     X   X   X   X X   X   X X X X X   X   X X X X X X X       X   X X X \n" +
          "X X X X       X       X   X X   X   X       X   X   X     X X X     X X       X X \n" +
          "X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n" +
          "    X     X       X         X   X   X       X   X   X     X   X X                 \n" +
          "        X X     X X X X X   X   X   X X X X X   X   X X X     X X X X   X         \n" +
          "X     X   X   X         X   X   X               X   X X   X X   X X X     X   X   \n" +
          "  X   X X X   X   X X   X X X   X X X X X X X X X   X X         X X     X X X X   \n" +
          "    X X   X   X   X X X     X                       X X X   X X   X   X     X     \n" +
          "    X X X X   X         X   X X X X X X X X X X X X X X   X       X X   X X   X X \n" +
          "            X   X   X X       X X X X X     X X X       X       X X X         X   \n" +
          "X       X         X   X X X X   X     X X     X X     X X           X   X       X \n" +
          "X     X       X X X X X     X   X X X X   X X X     X       X X X X   X   X X   X \n" +
          "  X X X X X               X     X X X   X       X X   X X   X X X X     X X       \n" +
          "X             X         X   X X   X X     X     X     X   X   X X X X             \n" +
          "    X   X X       X     X       X   X X X X X X   X X   X X X X X X X X X   X   X \n" +
          "    X         X X   X       X     X   X   X       X     X X X     X       X X X X \n" +
          "X     X X     X X X X X X             X X X   X               X   X     X     X X \n" +
          "X   X X     X               X X X X X     X X     X X X X X X X X     X   X   X X \n" +
          "X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n" +
          "X           X     X X X X     X     X         X         X   X       X X   X X X   \n" +
          "X   X   X X   X X X   X         X X     X X X X     X X   X   X     X   X       X \n" +
          "      X     X     X     X X     X   X X   X X   X         X X       X       X   X \n" +
          "X       X           X   X   X     X X   X               X     X     X X X         \n");
  }

  @Test
  public void testAztecWriter() throws Exception {
    for (int i = 0; i < 1000; i++) {
      testWriter("\u20AC 1 sample data.", "ISO-8859-1", 25, true, 2);
      testWriter("\u20AC 1 sample data.", "ISO-8859-15", 25, true, 2);
      testWriter("\u20AC 1 sample data.", "UTF-8", 25, true, 2);
      testWriter("\u20AC 1 sample data.", "UTF-8", 100, true, 3);
      testWriter("\u20AC 1 sample data.", "UTF-8", 300, true, 4);
      testWriter("\u20AC 1 sample data.", "UTF-8", 500, false, 5);
            String data = "In ut magna vel mauris malesuada";
      AztecWriter writer = new AztecWriter();
      BitMatrix matrix = writer.encode(data, BarcodeFormat.AZTEC, 0, 0);
      AztecCode aztec = Encoder.encode(data.getBytes(StandardCharsets.ISO_8859_1),
          Encoder.DEFAULT_EC_PERCENT, Encoder.DEFAULT_AZTEC_LAYERS);
      BitMatrix expectedMatrix = aztec.getMatrix();
      assertEquals(matrix, expectedMatrix);
    }
  }
  
  
  @Test
  public void testEncodeDecode1() throws Exception {
    testEncodeDecode("Abc123!", true, 1);
  }
  
  @Test
  public void testEncodeDecode2() throws Exception {
    testEncodeDecode("Lorem ipsum. http:  }
  
  @Test
  public void testEncodeDecode3() throws Exception {
    testEncodeDecode("AAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAAN", true, 3);
  }
  
  @Test
  public void testEncodeDecode4() throws Exception {
    testEncodeDecode("http:  }
  
  @Test
  public void testEncodeDecode5() throws Exception {
    testEncodeDecode("http:        + "Four score and seven our forefathers brought forth", false, 5);
  }
  
  @Test
  public void testEncodeDecode10() throws Exception {
    testEncodeDecode("In ut magna vel mauris malesuada dictum. Nulla ullamcorper metus quis diam" +
        " cursus facilisis. Sed mollis quam id justo rutrum sagittis. Donec laoreet rutrum" +
        " est, nec convallis mauris condimentum sit amet. Phasellus gravida, justo et congue" +
        " auctor, nisi ipsum viverra erat, eget hendrerit felis turpis nec lorem. Nulla" +
        " ultrices, elit pellentesque aliquet laoreet, justo erat pulvinar nisi, id" +
        " elementum sapien dolor et diam.", false, 10);
  }
  
  @Test
  public void testEncodeDecode23() throws Exception {
    testEncodeDecode("In ut magna vel mauris malesuada dictum. Nulla ullamcorper metus quis diam" +
        " cursus facilisis. Sed mollis quam id justo rutrum sagittis. Donec laoreet rutrum" +
        " est, nec convallis mauris condimentum sit amet. Phasellus gravida, justo et congue" +
        " auctor, nisi ipsum viverra erat, eget hendrerit felis turpis nec lorem. Nulla" +
        " ultrices, elit pellentesque aliquet laoreet, justo erat pulvinar nisi, id" +
        " elementum sapien dolor et diam. Donec ac nunc sodales elit placerat eleifend." +
        " Sed ornare luctus ornare. Vestibulum vehicula, massa at pharetra fringilla, risus" +
        " justo faucibus erat, nec porttitor nibh tellus sed est. Ut justo diam, lobortis eu" +
        " tristique ac, p.In ut magna vel mauris malesuada dictum. Nulla ullamcorper metus" +
        " quis diam cursus facilisis. Sed mollis quam id justo rutrum sagittis. Donec" +
        " laoreet rutrum est, nec convallis mauris condimentum sit amet. Phasellus gravida," +
        " justo et congue auctor, nisi ipsum viverra erat, eget hendrerit felis turpis nec" +
        " lorem. Nulla ultrices, elit pellentesque aliquet laoreet, justo erat pulvinar" +
        " nisi, id elementum sapien dolor et diam. Donec ac nunc sodales elit placerat" +
        " eleifend. Sed ornare luctus ornare. Vestibulum vehicula, massa at pharetra" +
        " fringilla, risus justo faucibus erat, nec porttitor nibh tellus sed est. Ut justo" +
        " diam, lobortis eu tristique ac, p. In ut magna vel mauris malesuada dictum. Nulla" +
        " ullamcorper metus quis diam cursus facilisis. Sed mollis quam id justo rutrum" +
        " sagittis. Donec laoreet rutrum est, nec convallis mauris condimentum sit amet." +
        " Phasellus gravida, justo et congue auctor, nisi ipsum viverra erat, eget hendrerit" +
        " felis turpis nec lorem. Nulla ultrices, elit pellentesque aliquet laoreet, justo" +
        " erat pulvinar nisi, id elementum sapien dolor et diam.", false, 23);
  }

  @Test
  public void testEncodeDecode31() throws Exception {
    testEncodeDecode("In ut magna vel mauris malesuada dictum. Nulla ullamcorper metus quis diam" +
      " cursus facilisis. Sed mollis quam id justo rutrum sagittis. Donec laoreet rutrum" +
      " est, nec convallis mauris condimentum sit amet. Phasellus gravida, justo et congue" +
      " auctor, nisi ipsum viverra erat, eget hendrerit felis turpis nec lorem. Nulla" +
      " ultrices, elit pellentesque aliquet laoreet, justo erat pulvinar nisi, id" +
      " elementum sapien dolor et diam. Donec ac nunc sodales elit placerat eleifend." +
      " Sed ornare luctus ornare. Vestibulum vehicula, massa at pharetra fringilla, risus" +
      " justo faucibus erat, nec porttitor nibh tellus sed est. Ut justo diam, lobortis eu" +
      " tristique ac, p.In ut magna vel mauris malesuada dictum. Nulla ullamcorper metus" +
      " quis diam cursus facilisis. Sed mollis quam id justo rutrum sagittis. Donec" +
      " laoreet rutrum est, nec convallis mauris condimentum sit amet. Phasellus gravida," +
      " justo et congue auctor, nisi ipsum viverra erat, eget hendrerit felis turpis nec" +
      " lorem. Nulla ultrices, elit pellentesque aliquet laoreet, justo erat pulvinar" +
      " nisi, id elementum sapien dolor et diam. Donec ac nunc sodales elit placerat" +
      " eleifend. Sed ornare luctus ornare. Vestibulum vehicula, massa at pharetra" +
      " fringilla, risus justo faucibus erat, nec porttitor nibh tellus sed est. Ut justo" +
      " diam, lobortis eu tristique ac, p. In ut magna vel mauris malesuada dictum. Nulla" +
      " ullamcorper metus quis diam cursus facilisis. Sed mollis quam id justo rutrum" +
      " sagittis. Donec laoreet rutrum est, nec convallis mauris condimentum sit amet." +
      " Phasellus gravida, justo et congue auctor, nisi ipsum viverra erat, eget hendrerit" +
      " felis turpis nec lorem. Nulla ultrices, elit pellentesque aliquet laoreet, justo" +
      " erat pulvinar nisi, id elementum sapien dolor et diam. Donec ac nunc sodales elit" +
      " placerat eleifend. Sed ornare luctus ornare. Vestibulum vehicula, massa at" +
      " pharetra fringilla, risus justo faucibus erat, nec porttitor nibh tellus sed est." +
      " Ut justo diam, lobortis eu tristique ac, p.In ut magna vel mauris malesuada" +
      " dictum. Nulla ullamcorper metus quis diam cursus facilisis. Sed mollis quam id" +
      " justo rutrum sagittis. Donec laoreet rutrum est, nec convallis mauris condimentum" +
      " sit amet. Phasellus gravida, justo et congue auctor, nisi ipsum viverra erat," +
      " eget hendrerit felis turpis nec lorem. Nulla ultrices, elit pellentesque aliquet" +
      " laoreet, justo erat pulvinar nisi, id elementum sapien dolor et diam. Donec ac" +
      " nunc sodales elit placerat eleifend. Sed ornare luctus ornare. Vestibulum vehicula," +
      " massa at pharetra fringilla, risus justo faucibus erat, nec porttitor nibh tellus" +
      " sed est. Ut justo diam, lobortis eu tris. In ut magna vel mauris malesuada dictum." +
      " Nulla ullamcorper metus quis diam cursus facilisis. Sed mollis quam id justo rutrum" +
      " sagittis. Donec laoreet rutrum est, nec convallis mauris condimentum sit amet." +
      " Phasellus gravida, justo et congue auctor, nisi ipsum viverra erat, eget" +
      " hendrerit felis turpis nec lorem.", false, 31);
  }

  @Test
  public void testGenerateModeMessage() {
    testModeMessage(true, 2, 29, ".X .XXX.. ...X XX.. ..X .XX. .XX.X");
    testModeMessage(true, 4, 64, "XX XXXXXX .X.. ...X ..XX .X.. XX..");
    testModeMessage(false, 21, 660,  "X.X.. .X.X..X..XX .XXX ..X.. .XXX. .X... ..XXX");
    testModeMessage(false, 32, 4096, "XXXXX XXXXXXXXXXX X.X. ..... XXX.X ..X.. X.XXX");
  }

  @Test
  public void testStuffBits() {
    testStuffBits(5, ".X.X. X.X.X .X.X.",
        ".X.X. X.X.X .X.X.");
    testStuffBits(5, ".X.X. ..... .X.X",
        ".X.X. ....X ..X.X");
    testStuffBits(3, "XX. ... ... ..X XXX .X. ..",
        "XX. ..X ..X ..X ..X .XX XX. .X. ..X");
    testStuffBits(6, ".X.X.. ...... ..X.XX",
        ".X.X.. .....X. ..X.XX XXXX.");
    testStuffBits(6, ".X.X.. ...... ...... ..X.X.",
        ".X.X.. .....X .....X ....X. X.XXXX");
    testStuffBits(6, ".X.X.. XXXXXX ...... ..X.XX",
        ".X.X.. XXXXX. X..... ...X.X XXXXX.");
    testStuffBits(6,
        "...... ..XXXX X..XX. .X.... .X.X.X .....X .X.... ...X.X .....X ....XX ..X... ....X. X..XXX X.XX.X",
        ".....X ...XXX XX..XX ..X... ..X.X. X..... X.X... ....X. X..... X....X X..X.. .....X X.X..X XXX.XX .XXXXX");
  }

  @Test
  public void testHighLevelEncode() throws Exception {
    testHighLevelEncodeString("A. b.",
                "...X. ..... ...XX XXX.. ...XX XXXX. XX.X");
    testHighLevelEncodeString("Lorem ipsum.",
                ".XX.X XXX.. X.... X..XX ..XX. .XXX. ....X .X.X. X...X X.X.. X.XX. .XXX. XXXX. XX.X");
    testHighLevelEncodeString("Lo. Test 123.",
                ".XX.X XXX.. X.... ..... ...XX XXX.. X.X.X ..XX. X.X.. X.X.X  XXXX. ...X ..XX .X.. .X.X XX.X");
    testHighLevelEncodeString("Lo...x",
                ".XX.X XXX.. X.... XXXX. XX.X XX.X XX.X XXX. XXX.. XX..X");
    testHighLevelEncodeString(". x:                "..... ...XX XXX.. XX..X ..... X.X.X ..... X.X.. ..... X.X.. ...X. ...XX ..X.. ..... X.X.. XXXX. XX.X");
        testHighLevelEncodeString("ABCdEFG",
                "...X. ...XX ..X.. XXXXX ....X .XX..X.. ..XX. ..XXX .X...");

    testHighLevelEncodeString(
                        "09  UAG    ^160MEUCIQC0sYS/HpKxnBELR1uB85R20OoqqwFGa0q2uEi"
            + "Ygh6utAIgLl1aBVM4EOTQtMQQYH9M2Z3Dp4qnA/fwWuQ+M8L3V8U=",
        823);
  }

  @Test
  public void testHighLevelEncodeBinary() throws Exception {
        testHighLevelEncodeString("N\0N",
                ".XXXX XXXXX ....X ........ .XXXX");   
    testHighLevelEncodeString("N\0n",
                ".XXXX XXXXX ...X. ........ .XX.XXX.");   
        testHighLevelEncodeString("N\0\u0080 A",
                ".XXXX XXXXX ...X. ........ X....... ....X ...X.");

        testHighLevelEncodeString("\0a\u00FF\u0080 A",
                "XXXXX ..X.. ........ .XX....X XXXXXXXX X....... ....X ...X.");

        testHighLevelEncodeString("1234\0",
                "XXXX. ..XX .X.. .X.X .XX. XXX. XXXXX ....X ........"
    );

        StringBuilder sb = new StringBuilder();
    for (int i = 0; i <= 3000; i++) {
      sb.append((char)(128 + (i % 30)));
    }
            for (int i : new int[] { 1, 2, 3, 10, 29, 30, 31, 32, 33,
                             60, 61, 62, 63, 64, 2076, 2077, 2078, 2079, 2080, 2100 }) {
            int expectedLength = (8 * i) +
          ( (i <= 31) ? 10 : (i <= 62) ? 20 : (i <= 2078) ? 21 : 31);
            testHighLevelEncodeString(sb.substring(0, i), expectedLength);
      if (i != 1 && i != 32 && i != 2079) {
                                        testHighLevelEncodeString('a' + sb.substring(0, i - 1), expectedLength);
                testHighLevelEncodeString(sb.substring(0, i - 1) + 'a', expectedLength);
      }
            testHighLevelEncodeString('a' + sb.substring(0, i) + 'b', expectedLength + 15);
    }
  }
  
  @Test
  public void testHighLevelEncodePairs() throws Exception {
        testHighLevelEncodeString("ABC. DEF\r\n",
                "...X. ...XX ..X.. ..... ...XX ..X.X ..XX. ..XXX ..... ...X.");

        testHighLevelEncodeString("A. : , \r\n",
                "...X. XXX.X XXXX. ...XX ..X.X  ..X.. ...X.");

        testHighLevelEncodeString("A. 1234",
                "...X. XXXX. XX.X ...X ..XX .X.. .X.X .X X."
        );
        testHighLevelEncodeString("A\200. \200",
                "...X. XXXXX ..X.. X....... ..X.XXX. ..X..... X.......");
  }

  @Test
  public void testUserSpecifiedLayers() throws Exception {
    byte[] alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ".getBytes(StandardCharsets.ISO_8859_1);
    AztecCode aztec = Encoder.encode(alphabet, 25, -2);
    assertEquals(2, aztec.getLayers());
    assertTrue(aztec.isCompact());

    aztec = Encoder.encode(alphabet, 25, 32);
    assertEquals(32, aztec.getLayers());
    assertFalse(aztec.isCompact());

    try {
      Encoder.encode(alphabet, 25, 33);
      fail("Encode should have failed.  No such thing as 33 layers");
    } catch (IllegalArgumentException expected) {
          }

    try {
      Encoder.encode(alphabet, 25, -1);
      fail("Encode should have failed.  Text can't fit in 1-layer compact");
    } catch (IllegalArgumentException expected) {
          }
  }

  @Test
  public void testBorderCompact4Case() throws Exception {
            String alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        String alphabet4 = alphabet + alphabet + alphabet + alphabet;
    byte[] data = alphabet4.getBytes(StandardCharsets.ISO_8859_1);
    try {
      Encoder.encode(data, 0, -4);
      fail("Encode should have failed.  Text can't fit in 1-layer compact");
    } catch (IllegalArgumentException expected) {
          }

        AztecCode aztecCode = Encoder.encode(data, 0, Encoder.DEFAULT_AZTEC_LAYERS);
    assertFalse(aztecCode.isCompact());
    assertEquals(4, aztecCode.getLayers());

            aztecCode = Encoder.encode(alphabet4.substring(0, 100).getBytes(StandardCharsets.ISO_8859_1), 10, Encoder.DEFAULT_AZTEC_LAYERS);
    assertTrue(aztecCode.isCompact());
    assertEquals(4, aztecCode.getLayers());
  }

  
  private static void testEncode(String data, boolean compact, int layers, String expected) throws Exception {
    AztecCode aztec = Encoder.encode(data.getBytes(StandardCharsets.ISO_8859_1), 33, Encoder.DEFAULT_AZTEC_LAYERS);
    assertEquals("Unexpected symbol format (compact)", compact, aztec.isCompact());
    assertEquals("Unexpected nr. of layers", layers, aztec.getLayers());
    BitMatrix matrix = aztec.getMatrix();
    assertEquals("encode() failed", expected, matrix.toString());
  }

  private static void testEncodeDecode(String data, boolean compact, int layers) throws Exception {
    AztecCode aztec = Encoder.encode(data.getBytes(StandardCharsets.ISO_8859_1), 25, Encoder.DEFAULT_AZTEC_LAYERS);
    assertEquals("Unexpected symbol format (compact)", compact, aztec.isCompact());
    assertEquals("Unexpected nr. of layers", layers, aztec.getLayers());
    BitMatrix matrix = aztec.getMatrix();
    AztecDetectorResult r = 
        new AztecDetectorResult(matrix, NO_POINTS, aztec.isCompact(), aztec.getCodeWords(), aztec.getLayers());
    DecoderResult res = new Decoder().decode(r);
    assertEquals(data, res.getText());
        Random random = getPseudoRandom();
    matrix.flip(random.nextInt(matrix.getWidth()), random.nextInt(2));
    matrix.flip(random.nextInt(matrix.getWidth()), matrix.getHeight() - 2 + random.nextInt(2));
    matrix.flip(random.nextInt(2), random.nextInt(matrix.getHeight()));
    matrix.flip(matrix.getWidth() - 2 + random.nextInt(2), random.nextInt(matrix.getHeight()));
    r = new AztecDetectorResult(matrix, NO_POINTS, aztec.isCompact(), aztec.getCodeWords(), aztec.getLayers());
    res = new Decoder().decode(r);
    assertEquals(data, res.getText());
  }

  private static void testWriter(String data, 
                                 String charset, 
                                 int eccPercent, 
                                 boolean compact, 
                                 int layers) throws FormatException {
            String expectedData = new String(data.getBytes(Charset.forName(charset)), StandardCharsets.ISO_8859_1);
    Map<EncodeHintType,Object> hints = new EnumMap<>(EncodeHintType.class);
    hints.put(EncodeHintType.CHARACTER_SET, charset);
    hints.put(EncodeHintType.ERROR_CORRECTION, eccPercent);
    AztecWriter writer = new AztecWriter();
    BitMatrix matrix = writer.encode(data, BarcodeFormat.AZTEC, 0, 0, hints);
    AztecCode aztec = Encoder.encode(data.getBytes(Charset.forName(charset)), eccPercent,
        Encoder.DEFAULT_AZTEC_LAYERS);
    assertEquals("Unexpected symbol format (compact)", compact, aztec.isCompact());
    assertEquals("Unexpected nr. of layers", layers, aztec.getLayers());
    BitMatrix matrix2 = aztec.getMatrix();
    assertEquals(matrix, matrix2);
    AztecDetectorResult r = 
        new AztecDetectorResult(matrix, NO_POINTS, aztec.isCompact(), aztec.getCodeWords(), aztec.getLayers());
    DecoderResult res = new Decoder().decode(r);
    assertEquals(expectedData, res.getText());
        int ecWords = aztec.getCodeWords() * eccPercent / 100 / 2;
    Random random = getPseudoRandom();
    for (int i = 0; i < ecWords; i++) {
            int x = random.nextBoolean() ?
                random.nextInt(aztec.getLayers() * 2)
                : matrix.getWidth() - 1 - random.nextInt(aztec.getLayers() * 2);
      int y = random.nextBoolean() ?
                random.nextInt(aztec.getLayers() * 2)
                : matrix.getHeight() - 1 - random.nextInt(aztec.getLayers() * 2);
      matrix.flip(x, y);
    }
    r = new AztecDetectorResult(matrix, NO_POINTS, aztec.isCompact(), aztec.getCodeWords(), aztec.getLayers());
    res = new Decoder().decode(r);
    assertEquals(expectedData, res.getText());
  }

  private static Random getPseudoRandom() {
    return new Random(0xDEADBEEF);
  }

  private static void testModeMessage(boolean compact, int layers, int words, String expected) {
    BitArray in = Encoder.generateModeMessage(compact, layers, words);
    assertEquals("generateModeMessage() failed", stripSpace(expected), stripSpace(in.toString()));
  }

  private static void testStuffBits(int wordSize, String bits, String expected) {
    BitArray in = toBitArray(bits);
    BitArray stuffed = Encoder.stuffBits(in, wordSize);
    assertEquals("stuffBits() failed for input string: " + bits, 
                 stripSpace(expected), stripSpace(stuffed.toString()));
  }

  private static BitArray toBitArray(CharSequence bits) {
    BitArray in = new BitArray();
    char[] str = DOTX.matcher(bits).replaceAll("").toCharArray();
    for (char aStr : str) {
      in.appendBit(aStr == 'X');
    }
    return in;
  }

  private static boolean[] toBooleanArray(BitArray bitArray) {
    boolean[] result = new boolean[bitArray.getSize()];
    for (int i = 0; i < result.length; i++) {
      result[i] = bitArray.get(i);
    }
    return result;
  }

  private static void testHighLevelEncodeString(String s, String expectedBits) {
    BitArray bits = new HighLevelEncoder(s.getBytes(StandardCharsets.ISO_8859_1)).encode();
    String receivedBits = stripSpace(bits.toString());
    assertEquals("highLevelEncode() failed for input string: " + s, stripSpace(expectedBits), receivedBits);
    assertEquals(s, Decoder.highLevelDecode(toBooleanArray(bits)));
  }

  private static void testHighLevelEncodeString(String s, int expectedReceivedBits) {
    BitArray bits = new HighLevelEncoder(s.getBytes(StandardCharsets.ISO_8859_1)).encode();
    int receivedBitCount = stripSpace(bits.toString()).length();
    assertEquals("highLevelEncode() failed for input string: " + s, 
                 expectedReceivedBits, receivedBitCount);
    assertEquals(s, Decoder.highLevelDecode(toBooleanArray(bits)));
  }

  private static String stripSpace(String s) {
    return SPACES.matcher(s).replaceAll("");
  }

}
