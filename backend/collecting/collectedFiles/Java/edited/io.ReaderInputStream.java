

package com.google.common.io;

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.base.Preconditions.checkPositionIndexes;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.primitives.UnsignedBytes;
import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;
import java.nio.charset.CodingErrorAction;
import java.util.Arrays;


@GwtIncompatible
final class ReaderInputStream extends InputStream {
  private final Reader reader;
  private final CharsetEncoder encoder;
  private final byte[] singleByte = new byte[1];

  
  private CharBuffer charBuffer;

  
  private ByteBuffer byteBuffer;

  
  private boolean endOfInput;
  
  private boolean draining;
  
  private boolean doneFlushing;

  
  ReaderInputStream(Reader reader, Charset charset, int bufferSize) {
    this(
        reader,
        charset
            .newEncoder()
            .onMalformedInput(CodingErrorAction.REPLACE)
            .onUnmappableCharacter(CodingErrorAction.REPLACE),
        bufferSize);
  }

  
  ReaderInputStream(Reader reader, CharsetEncoder encoder, int bufferSize) {
    this.reader = checkNotNull(reader);
    this.encoder = checkNotNull(encoder);
    checkArgument(bufferSize > 0, "bufferSize must be positive: %s", bufferSize);
    encoder.reset();

    charBuffer = CharBuffer.allocate(bufferSize);
    charBuffer.flip();

    byteBuffer = ByteBuffer.allocate(bufferSize);
  }

  @Override
  public void close() throws IOException {
    reader.close();
  }

  @Override
  public int read() throws IOException {
    return (read(singleByte) == 1) ? UnsignedBytes.toInt(singleByte[0]) : -1;
  }

      @Override
  public int read(byte[] b, int off, int len) throws IOException {
        checkPositionIndexes(off, off + len, b.length);
    if (len == 0) {
      return 0;
    }

        int totalBytesRead = 0;
    boolean doneEncoding = endOfInput;

    DRAINING:
    while (true) {
                  if (draining) {
        totalBytesRead += drain(b, off + totalBytesRead, len - totalBytesRead);
        if (totalBytesRead == len || doneFlushing) {
          return (totalBytesRead > 0) ? totalBytesRead : -1;
        }
        draining = false;
        byteBuffer.clear();
      }

      while (true) {
                        CoderResult result;
        if (doneFlushing) {
          result = CoderResult.UNDERFLOW;
        } else if (doneEncoding) {
          result = encoder.flush(byteBuffer);
        } else {
          result = encoder.encode(charBuffer, byteBuffer, endOfInput);
        }

        if (result.isOverflow()) {
                    startDraining(true);
          continue DRAINING;
        } else if (result.isUnderflow()) {
                                                  if (doneEncoding) {             doneFlushing = true;
            startDraining(false);
            continue DRAINING;
          } else if (endOfInput) {             doneEncoding = true;
          } else {             readMoreChars();
          }
        } else if (result.isError()) {
                    result.throwException();
          return 0;         }
      }
    }
  }

  
  private static CharBuffer grow(CharBuffer buf) {
    char[] copy = Arrays.copyOf(buf.array(), buf.capacity() * 2);
    CharBuffer bigger = CharBuffer.wrap(copy);
    bigger.position(buf.position());
    bigger.limit(buf.limit());
    return bigger;
  }

  
  private void readMoreChars() throws IOException {
                            
    if (availableCapacity(charBuffer) == 0) {
      if (charBuffer.position() > 0) {
                charBuffer.compact().flip();
      } else {
                charBuffer = grow(charBuffer);
      }
    }

        int limit = charBuffer.limit();
    int numChars = reader.read(charBuffer.array(), limit, availableCapacity(charBuffer));
    if (numChars == -1) {
      endOfInput = true;
    } else {
      charBuffer.limit(limit + numChars);
    }
  }

  
  private static int availableCapacity(Buffer buffer) {
    return buffer.capacity() - buffer.limit();
  }

  
  private void startDraining(boolean overflow) {
    byteBuffer.flip();
    if (overflow && byteBuffer.remaining() == 0) {
      byteBuffer = ByteBuffer.allocate(byteBuffer.capacity() * 2);
    } else {
      draining = true;
    }
  }

  
  private int drain(byte[] b, int off, int len) {
    int remaining = Math.min(len, byteBuffer.remaining());
    byteBuffer.get(b, off, remaining);
    return remaining;
  }
}
