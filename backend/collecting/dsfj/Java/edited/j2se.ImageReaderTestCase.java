

package com.google.zxing.client.j2se;

import org.junit.Assert;
import org.junit.Test;

import java.awt.image.BufferedImage;
import java.net.URI;

public final class ImageReaderTestCase extends Assert {
  
  @Test
  public void testFoo() throws Exception {
    String uri = 
        "data:image/gif;base64,R0lGODlhEAAQAMQAAORHHOVSKudfOulrSOp3WOyDZu6QdvCchPGolfO0o/XBs/fNwfjZ0frl3/zy7        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACH5BAkAABAALAAAAAAQABAAAAVVICSOZGlCQAosJ6" +
        "mu7fiyZeKqNKToQGDsM8hBADgUXoGAiqhSvp5QAnQKGIgUhwFUYLCVDFCrKUE1lBavAViFIDlTImbKC5Gm2hB0SlBCBMQiB0UjIQA7";
    BufferedImage image = ImageReader.readImage(new URI(uri));
    assertEquals(16, image.getWidth());
    assertEquals(16, image.getHeight());
  }
  
}
