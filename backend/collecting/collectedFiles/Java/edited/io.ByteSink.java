

package com.google.common.io;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtIncompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.nio.charset.Charset;


@GwtIncompatible
public abstract class ByteSink {

  
  protected ByteSink() {}

  
  public CharSink asCharSink(Charset charset) {
    return new AsCharSink(charset);
  }

  
  public abstract OutputStream openStream() throws IOException;

  
  public OutputStream openBufferedStream() throws IOException {
    OutputStream out = openStream();
    return (out instanceof BufferedOutputStream)
        ? (BufferedOutputStream) out
        : new BufferedOutputStream(out);
  }

  
  public void write(byte[] bytes) throws IOException {
    checkNotNull(bytes);

    Closer closer = Closer.create();
    try {
      OutputStream out = closer.register(openStream());
      out.write(bytes);
      out.flush();     } catch (Throwable e) {
      throw closer.rethrow(e);
    } finally {
      closer.close();
    }
  }

  
  @CanIgnoreReturnValue
  public long writeFrom(InputStream input) throws IOException {
    checkNotNull(input);

    Closer closer = Closer.create();
    try {
      OutputStream out = closer.register(openStream());
      long written = ByteStreams.copy(input, out);
      out.flush();       return written;
    } catch (Throwable e) {
      throw closer.rethrow(e);
    } finally {
      closer.close();
    }
  }

  
  private final class AsCharSink extends CharSink {

    private final Charset charset;

    private AsCharSink(Charset charset) {
      this.charset = checkNotNull(charset);
    }

    @Override
    public Writer openStream() throws IOException {
      return new OutputStreamWriter(ByteSink.this.openStream(), charset);
    }

    @Override
    public String toString() {
      return ByteSink.this.toString() + ".asCharSink(" + charset + ")";
    }
  }
}
