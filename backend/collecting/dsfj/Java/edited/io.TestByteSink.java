

package com.google.common.io;

import com.google.common.collect.ImmutableSet;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;


public class TestByteSink extends ByteSink implements TestStreamSupplier {

  private final ByteArrayOutputStream bytes = new ByteArrayOutputStream();
  private final ImmutableSet<TestOption> options;

  private boolean outputStreamOpened;
  private boolean outputStreamClosed;

  public TestByteSink(TestOption... options) {
    this.options = ImmutableSet.copyOf(options);
  }

  byte[] getBytes() {
    return bytes.toByteArray();
  }

  @Override
  public boolean wasStreamOpened() {
    return outputStreamOpened;
  }

  @Override
  public boolean wasStreamClosed() {
    return outputStreamClosed;
  }

  @Override
  public OutputStream openStream() throws IOException {
    outputStreamOpened = true;
    bytes.reset();     return new Out();
  }

  private final class Out extends TestOutputStream {

    public Out() throws IOException {
      super(bytes, options);
    }

    @Override
    public void close() throws IOException {
      outputStreamClosed = true;
      super.close();
    }
  }
}
