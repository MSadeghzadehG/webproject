

package com.google.common.io;

import static com.google.common.base.Charsets.UTF_8;
import static com.google.common.base.Preconditions.checkNotNull;

import java.io.FilterWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;


public class TestWriter extends FilterWriter {

  private final TestOutputStream out;

  public TestWriter(TestOption... options) throws IOException {
    this(new TestOutputStream(ByteStreams.nullOutputStream(), options));
  }

  public TestWriter(TestOutputStream out) {
    super(new OutputStreamWriter(checkNotNull(out), UTF_8));
    this.out = out;
  }

  @Override
  public void write(int c) throws IOException {
    super.write(c);
    flush();   }

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

  public boolean closed() {
    return out.closed();
  }
}
