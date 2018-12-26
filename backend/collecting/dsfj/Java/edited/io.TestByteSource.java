

package com.google.common.io;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.collect.ImmutableSet;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Random;


public final class TestByteSource extends ByteSource implements TestStreamSupplier {

  private final byte[] bytes;
  private final ImmutableSet<TestOption> options;

  private boolean inputStreamOpened;
  private boolean inputStreamClosed;

  TestByteSource(byte[] bytes, TestOption... options) {
    this.bytes = checkNotNull(bytes);
    this.options = ImmutableSet.copyOf(options);
  }

  @Override
  public boolean wasStreamOpened() {
    return inputStreamOpened;
  }

  @Override
  public boolean wasStreamClosed() {
    return inputStreamClosed;
  }

  @Override
  public InputStream openStream() throws IOException {
    inputStreamOpened = true;
    return new RandomAmountInputStream(new In(), new Random());
  }

  private final class In extends TestInputStream {

    public In() throws IOException {
      super(new ByteArrayInputStream(bytes), options);
    }

    @Override
    public void close() throws IOException {
      inputStreamClosed = true;
      super.close();
    }
  }
}
