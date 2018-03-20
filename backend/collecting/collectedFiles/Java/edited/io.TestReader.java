

package com.google.common.io;

import static com.google.common.base.Charsets.UTF_8;
import static com.google.common.base.Preconditions.checkNotNull;

import java.io.ByteArrayInputStream;
import java.io.FilterReader;
import java.io.IOException;
import java.io.InputStreamReader;


public class TestReader extends FilterReader {

  private final TestInputStream in;

  public TestReader(TestOption... options) throws IOException {
    this(new TestInputStream(new ByteArrayInputStream(new byte[10]), options));
  }

  public TestReader(TestInputStream in) {
    super(new InputStreamReader(checkNotNull(in), UTF_8));
    this.in = in;
  }

  public boolean closed() {
    return in.closed();
  }
}
