

package com.google.common.hash;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.Beta;
import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.OutputStream;


@Beta
public final class HashingOutputStream extends FilterOutputStream {
  private final Hasher hasher;

  
          public HashingOutputStream(HashFunction hashFunction, OutputStream out) {
    super(checkNotNull(out));
    this.hasher = checkNotNull(hashFunction.newHasher());
  }

  @Override
  public void write(int b) throws IOException {
    hasher.putByte((byte) b);
    out.write(b);
  }

  @Override
  public void write(byte[] bytes, int off, int len) throws IOException {
    hasher.putBytes(bytes, off, len);
    out.write(bytes, off, len);
  }

  
  public HashCode hash() {
    return hasher.hash();
  }

        @Override
  public void close() throws IOException {
    out.close();
  }
}
