

package com.google.common.io;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.io.TestOption.CLOSE_THROWS;
import static com.google.common.io.TestOption.OPEN_THROWS;
import static com.google.common.io.TestOption.READ_THROWS;
import static com.google.common.io.TestOption.SKIP_THROWS;

import com.google.common.collect.ImmutableSet;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;


public class TestInputStream extends FilterInputStream {

  private final ImmutableSet<TestOption> options;
  private boolean closed;

  public TestInputStream(InputStream in, TestOption... options) throws IOException {
    this(in, Arrays.asList(options));
  }

  public TestInputStream(InputStream in, Iterable<TestOption> options) throws IOException {
    super(checkNotNull(in));
    this.options = ImmutableSet.copyOf(options);
    throwIf(OPEN_THROWS);
  }

  public boolean closed() {
    return closed;
  }

  @Override
  public int read() throws IOException {
    throwIf(closed);
    throwIf(READ_THROWS);
    return in.read();
  }

  @Override
  public int read(byte[] b, int off, int len) throws IOException {
    throwIf(closed);
    throwIf(READ_THROWS);
    return in.read(b, off, len);
  }

  @Override
  public long skip(long n) throws IOException {
    throwIf(closed);
    throwIf(SKIP_THROWS);
    return in.skip(n);
  }

  @Override
  public int available() throws IOException {
    throwIf(closed);
    return options.contains(TestOption.AVAILABLE_ALWAYS_ZERO) ? 0 : in.available();
  }

  @Override
  public void close() throws IOException {
    closed = true;
    throwIf(CLOSE_THROWS);
    in.close();
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
