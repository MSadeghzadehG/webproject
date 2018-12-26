

package com.google.zxing.client.j2se;

import org.junit.Assert;
import org.junit.Test;

import java.nio.charset.StandardCharsets;

public final class Base64DecoderTestCase extends Assert {
  
  @Test
  public void testEncode() {
    Base64Decoder decoder = Base64Decoder.getInstance();
    assertArrayEquals("foo".getBytes(StandardCharsets.UTF_8), decoder.decode("Zm9v"));
  }
  
}
