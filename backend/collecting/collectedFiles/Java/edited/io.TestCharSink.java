

package com.google.common.io;

import static com.google.common.base.Charsets.UTF_8;

import java.io.FilterWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;


public class TestCharSink extends CharSink implements TestStreamSupplier {

  private final TestByteSink byteSink;

  public TestCharSink(TestOption... options) {
    this.byteSink = new TestByteSink(options);
  }

  public String getString() {
    return new String(byteSink.getBytes(), UTF_8);
  }

  @Override
  public boolean wasStreamOpened() {
    return byteSink.wasStreamOpened();
  }

  @Override
  public boolean wasStreamClosed() {
    return byteSink.wasStreamClosed();
  }

  @Override
  public Writer openStream() throws IOException {
        return new FilterWriter(new OutputStreamWriter(byteSink.openStream(), UTF_8)) {
      @Override
      public void write(int c) throws IOException {
        super.write(c);
        flush();
      }

      @Override
      public void write(char[] cbuf, int off, int len) throws IOException {
        super.write(cbuf, off, len);
        flush();
      }

      @Override
      public void write(String str, int off, int len) throws IOException {
        super.write(str, off, len);
        flush();
      }
    };
  }
}
