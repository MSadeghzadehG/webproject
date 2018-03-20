

package com.google.common.io;

import static com.google.common.base.Charsets.UTF_8;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;


public class TestCharSource extends CharSource implements TestStreamSupplier {

  private final TestByteSource byteSource;

  public TestCharSource(String content, TestOption... options) {
    this.byteSource = new TestByteSource(content.getBytes(UTF_8), options);
  }

  @Override
  public boolean wasStreamOpened() {
    return byteSource.wasStreamOpened();
  }

  @Override
  public boolean wasStreamClosed() {
    return byteSource.wasStreamClosed();
  }

  @Override
  public Reader openStream() throws IOException {
    return new InputStreamReader(byteSource.openStream(), UTF_8);
  }
}
