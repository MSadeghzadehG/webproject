

package com.google.zxing.pdf417.encoder;

import java.nio.charset.StandardCharsets;

import org.junit.Assert;
import org.junit.Test;

public final class PDF417EncoderTestCase extends Assert {

  @Test
  public void testEncodeAuto() throws Exception {
    String encoded = PDF417HighLevelEncoder.encodeHighLevel(
        "ABCD", Compaction.AUTO, StandardCharsets.UTF_8);
    assertEquals("\u039f\u001A\u0385ABCD", encoded);
  }

  @Test
  public void testEncodeAutoWithSpecialChars() throws Exception {
	    PDF417HighLevelEncoder.encodeHighLevel(
        "1%§s ?aG$", Compaction.AUTO, StandardCharsets.UTF_8);
  }
  
  @Test
  public void testEncodeIso88591WithSpecialChars() throws Exception {
	  	  PDF417HighLevelEncoder.encodeHighLevel("asdfg§asd", Compaction.AUTO, StandardCharsets.ISO_8859_1);
  }

  @Test
  public void testEncodeText() throws Exception {
    String encoded = PDF417HighLevelEncoder.encodeHighLevel(
        "ABCD", Compaction.TEXT, StandardCharsets.UTF_8);
    assertEquals("Ο\u001A\u0001?", encoded);
  }

  @Test
  public void testEncodeNumeric() throws Exception {
    String encoded = PDF417HighLevelEncoder.encodeHighLevel(
        "1234", Compaction.NUMERIC, StandardCharsets.UTF_8);
    assertEquals("\u039f\u001A\u0386\f\u01b2", encoded);
  }

  @Test
  public void testEncodeByte() throws Exception {
    String encoded = PDF417HighLevelEncoder.encodeHighLevel(
        "abcd", Compaction.BYTE, StandardCharsets.UTF_8);
    assertEquals("\u039f\u001A\u0385abcd", encoded);
  }

}
