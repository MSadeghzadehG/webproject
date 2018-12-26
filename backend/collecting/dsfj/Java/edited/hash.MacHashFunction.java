

package com.google.common.hash;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.base.Preconditions.checkState;

import com.google.errorprone.annotations.Immutable;
import java.nio.ByteBuffer;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.NoSuchAlgorithmException;
import javax.crypto.Mac;


@Immutable
final class MacHashFunction extends AbstractHashFunction {

  @SuppressWarnings("Immutable")   private final Mac prototype;
  @SuppressWarnings("Immutable")   private final Key key;
  private final String toString;
  private final int bits;
  private final boolean supportsClone;

  MacHashFunction(String algorithmName, Key key, String toString) {
    this.prototype = getMac(algorithmName, key);
    this.key = checkNotNull(key);
    this.toString = checkNotNull(toString);
    this.bits = prototype.getMacLength() * Byte.SIZE;
    this.supportsClone = supportsClone(prototype);
  }

  @Override
  public int bits() {
    return bits;
  }

  private static boolean supportsClone(Mac mac) {
    try {
      mac.clone();
      return true;
    } catch (CloneNotSupportedException e) {
      return false;
    }
  }

  private static Mac getMac(String algorithmName, Key key) {
    try {
      Mac mac = Mac.getInstance(algorithmName);
      mac.init(key);
      return mac;
    } catch (NoSuchAlgorithmException e) {
      throw new IllegalStateException(e);
    } catch (InvalidKeyException e) {
      throw new IllegalArgumentException(e);
    }
  }

  @Override
  public Hasher newHasher() {
    if (supportsClone) {
      try {
        return new MacHasher((Mac) prototype.clone());
      } catch (CloneNotSupportedException e) {
              }
    }
    return new MacHasher(getMac(prototype.getAlgorithm(), key));
  }

  @Override
  public String toString() {
    return toString;
  }

  
  private static final class MacHasher extends AbstractByteHasher {
    private final Mac mac;
    private boolean done;

    private MacHasher(Mac mac) {
      this.mac = mac;
    }

    @Override
    protected void update(byte b) {
      checkNotDone();
      mac.update(b);
    }

    @Override
    protected void update(byte[] b) {
      checkNotDone();
      mac.update(b);
    }

    @Override
    protected void update(byte[] b, int off, int len) {
      checkNotDone();
      mac.update(b, off, len);
    }

    @Override
    protected void update(ByteBuffer bytes) {
      checkNotDone();
      checkNotNull(bytes);
      mac.update(bytes);
    }

    private void checkNotDone() {
      checkState(!done, "Cannot re-use a Hasher after calling hash() on it");
    }

    @Override
    public HashCode hash() {
      checkNotDone();
      done = true;
      return HashCode.fromBytesNoCopy(mac.doFinal());
    }
  }
}
