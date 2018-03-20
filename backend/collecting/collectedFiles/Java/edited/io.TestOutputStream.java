

package com.google.common.io;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.io.TestOption.CLOSE_THROWS;
import static com.google.common.io.TestOption.OPEN_THROWS;
import static com.google.common.io.TestOption.WRITE_THROWS;

import com.google.common.collect.ImmutableSet;
import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Arrays;


public class TestOutputStream extends FilterOutputStream {

  private final ImmutableSet<TestOption> options;
  private boolean closed;

  public TestOutputStream(OutputStream out, TestOption... options) throws IOException {
    this(out, Arrays.asList(options));
  }

  public TestOutputStream(OutputStream out, Iterable<TestOption> options) throws IOException {
    super(checkNotNull(out));
    this.options = ImmutableSet.copyOf(options);
    throwIf(OPEN_THROWS);
  }

  public boolean closed() {
    return closed;
  }

  @Override
  public void write(byte[] b, int off, int len) throws IOException {
    throwIf(closed);
    throwIf(WRITE_THROWS);
    super.write(b, off, len);
  }

  @Override
  public void write(int b) throws IOException {
    throwIf(closed);
    throwIf(WRITE_THROWS);
    super.write(b);
  }

  @Override
  public void close() throws IOException {
    closed = true;
    super.close();
    throwIf(CLOSE_THROWS);
  }

  private void throwIf(TestOption option) throws IOException {
    throwIf(options.contains(option));
  }

  private static void throwIf(boolean condition) throws IOException {
    if (condition) {
      throw new IOException();
    }
  }
}
