

package com.google.common.io;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtIncompatible;
import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtIncompatible
final class MultiInputStream extends InputStream {

  private Iterator<? extends ByteSource> it;
  @NullableDecl private InputStream in;

  
  public MultiInputStream(Iterator<? extends ByteSource> it) throws IOException {
    this.it = checkNotNull(it);
    advance();
  }

  @Override
  public void close() throws IOException {
    if (in != null) {
      try {
        in.close();
      } finally {
        in = null;
      }
    }
  }

  
  private void advance() throws IOException {
    close();
    if (it.hasNext()) {
      in = it.next().openStream();
    }
  }

  @Override
  public int available() throws IOException {
    if (in == null) {
      return 0;
    }
    return in.available();
  }

  @Override
  public boolean markSupported() {
    return false;
  }

  @Override
  public int read() throws IOException {
    while (in != null) {
      int result = in.read();
      if (result != -1) {
        return result;
      }
      advance();
    }
    return -1;
  }

  @Override
  public int read(@NullableDecl byte[] b, int off, int len) throws IOException {
    while (in != null) {
      int result = in.read(b, off, len);
      if (result != -1) {
        return result;
      }
      advance();
    }
    return -1;
  }

  @Override
  public long skip(long n) throws IOException {
    if (in == null || n <= 0) {
      return 0;
    }
    long result = in.skip(n);
    if (result != 0) {
      return result;
    }
    if (read() == -1) {
      return 0;
    }
    return 1 + in.skip(n - 1);
  }
}
