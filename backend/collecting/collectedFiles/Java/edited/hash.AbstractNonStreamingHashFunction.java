

package com.google.common.hash;

import com.google.common.base.Preconditions;
import com.google.errorprone.annotations.Immutable;
import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.Charset;
import java.util.Arrays;


@Immutable
abstract class AbstractNonStreamingHashFunction extends AbstractHashFunction {
  @Override
  public Hasher newHasher() {
    return newHasher(32);
  }

  @Override
  public Hasher newHasher(int expectedInputSize) {
    Preconditions.checkArgument(expectedInputSize >= 0);
    return new BufferingHasher(expectedInputSize);
  }

  @Override
  public HashCode hashInt(int input) {
    return hashBytes(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(input).array());
  }

  @Override
  public HashCode hashLong(long input) {
    return hashBytes(ByteBuffer.allocate(8).order(ByteOrder.LITTLE_ENDIAN).putLong(input).array());
  }

  @Override
  public HashCode hashUnencodedChars(CharSequence input) {
    int len = input.length();
    ByteBuffer buffer = ByteBuffer.allocate(len * 2).order(ByteOrder.LITTLE_ENDIAN);
    for (int i = 0; i < len; i++) {
      buffer.putChar(input.charAt(i));
    }
    return hashBytes(buffer.array());
  }

  @Override
  public HashCode hashString(CharSequence input, Charset charset) {
    return hashBytes(input.toString().getBytes(charset));
  }

  @Override
  public abstract HashCode hashBytes(byte[] input, int off, int len);

  @Override
  public HashCode hashBytes(ByteBuffer input) {
    return newHasher(input.remaining()).putBytes(input).hash();
  }

  
  private final class BufferingHasher extends AbstractHasher {
    final ExposedByteArrayOutputStream stream;

    BufferingHasher(int expectedInputSize) {
      this.stream = new ExposedByteArrayOutputStream(expectedInputSize);
    }

    @Override
    public Hasher putByte(byte b) {
      stream.write(b);
      return this;
    }

    @Override
    public Hasher putBytes(byte[] bytes, int off, int len) {
      stream.write(bytes, off, len);
      return this;
    }

    @Override
    public Hasher putBytes(ByteBuffer bytes) {
      stream.write(bytes);
      return this;
    }

    @Override
    public HashCode hash() {
      return hashBytes(stream.byteArray(), 0, stream.length());
    }
  }

    private static final class ExposedByteArrayOutputStream extends ByteArrayOutputStream {
    ExposedByteArrayOutputStream(int expectedInputSize) {
      super(expectedInputSize);
    }

    void write(ByteBuffer input) {
      int remaining = input.remaining();
      if (count + remaining > buf.length) {
        buf = Arrays.copyOf(buf, count + remaining);
      }
      input.get(buf, count, remaining);
      count += remaining;
    }

    byte[] byteArray() {
      return buf;
    }

    int length() {
      return count;
    }
  }
}
