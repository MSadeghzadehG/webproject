

package com.google.common.io;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;


@Beta
@GwtIncompatible
public final class CountingInputStream extends FilterInputStream {

  private long count;
  private long mark = -1;

  
  public CountingInputStream(InputStream in) {
    super(checkNotNull(in));
  }

  
  public long getCount() {
    return count;
  }

  @Override
  public int read() throws IOException {
    int result = in.read();
    if (result != -1) {
      count++;
    }
    return result;
  }

  @Override
  public int read(byte[] b, int off, int len) throws IOException {
    int result = in.read(b, off, len);
    if (result != -1) {
      count += result;
    }
    return result;
  }

  @Override
  public long skip(long n) throws IOException {
    long result = in.skip(n);
    count += result;
    return result;
  }

  @Override
  public synchronized void mark(int readlimit) {
    in.mark(readlimit);
    mark = count;
      }

  @Override
  public synchronized void reset() throws IOException {
    if (!in.markSupported()) {
      throw new IOException("Mark not supported");
    }
    if (mark == -1) {
      throw new IOException("Mark not set");
    }

    in.reset();
    count = mark;
  }
}
