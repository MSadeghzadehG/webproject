

package com.google.common.io;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.OutputStream;


@Beta
@GwtIncompatible
public final class CountingOutputStream extends FilterOutputStream {

  private long count;

  
  public CountingOutputStream(OutputStream out) {
    super(checkNotNull(out));
  }

  
  public long getCount() {
    return count;
  }

  @Override
  public void write(byte[] b, int off, int len) throws IOException {
    out.write(b, off, len);
    count += len;
  }

  @Override
  public void write(int b) throws IOException {
    out.write(b);
    count++;
  }

        @Override
  public void close() throws IOException {
    out.close();
  }
}
