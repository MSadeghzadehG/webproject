

package com.google.common.hash;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.Beta;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;


@Beta
public final class HashingInputStream extends FilterInputStream {
  private final Hasher hasher;

  
  public HashingInputStream(HashFunction hashFunction, InputStream in) {
    super(checkNotNull(in));
    this.hasher = checkNotNull(hashFunction.newHasher());
  }

  
  @Override
  @CanIgnoreReturnValue
  public int read() throws IOException {
    int b = in.read();
    if (b != -1) {
      hasher.putByte((byte) b);
    }
    return b;
  }

  
  @Override
  @CanIgnoreReturnValue
  public int read(byte[] bytes, int off, int len) throws IOException {
    int numOfBytesRead = in.read(bytes, off, len);
    if (numOfBytesRead != -1) {
      hasher.putBytes(bytes, off, numOfBytesRead);
    }
    return numOfBytesRead;
  }

  
  @Override
  public boolean markSupported() {
    return false;
  }

  
  @Override
  public void mark(int readlimit) {}

  
  @Override
  public void reset() throws IOException {
    throw new IOException("reset not supported");
  }

  
  public HashCode hash() {
    return hasher.hash();
  }
}
