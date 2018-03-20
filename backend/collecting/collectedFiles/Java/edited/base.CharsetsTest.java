

package com.google.common.base;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import java.nio.charset.Charset;
import java.util.Arrays;
import junit.framework.TestCase;


@GwtCompatible(emulated = true)
public class CharsetsTest extends TestCase {

  @GwtIncompatible   public void testUsAscii() {
    assertEquals(Charset.forName("US-ASCII"), Charsets.US_ASCII);
  }

  @GwtIncompatible   public void testIso88591() {
    assertEquals(Charset.forName("ISO-8859-1"), Charsets.ISO_8859_1);
  }

  public void testUtf8() {
    assertEquals(Charset.forName("UTF-8"), Charsets.UTF_8);
  }

  @GwtIncompatible   public void testUtf16be() {
    assertEquals(Charset.forName("UTF-16BE"), Charsets.UTF_16BE);
  }

  @GwtIncompatible   public void testUtf16le() {
    assertEquals(Charset.forName("UTF-16LE"), Charsets.UTF_16LE);
  }

  @GwtIncompatible   public void testUtf16() {
    assertEquals(Charset.forName("UTF-16"), Charsets.UTF_16);
  }

  @GwtIncompatible   public void testWhyUsAsciiIsDangerous() {
    byte[] b1 = "朝日新聞".getBytes(Charsets.US_ASCII);
    byte[] b2 = "聞朝日新".getBytes(Charsets.US_ASCII);
    byte[] b3 = "????".getBytes(Charsets.US_ASCII);
    byte[] b4 = "ニュース".getBytes(Charsets.US_ASCII);
    byte[] b5 = "スューー".getBytes(Charsets.US_ASCII);
        assertTrue(Arrays.equals(b1, b2));
    assertTrue(Arrays.equals(b2, b3));
    assertTrue(Arrays.equals(b3, b4));
    assertTrue(Arrays.equals(b4, b5));
  }
}
