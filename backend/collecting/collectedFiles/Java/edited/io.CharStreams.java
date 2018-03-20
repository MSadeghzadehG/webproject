

package com.google.common.io;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.base.Preconditions.checkPositionIndexes;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.io.Closeable;
import java.io.EOFException;
import java.io.IOException;
import java.io.Reader;
import java.io.Writer;
import java.nio.CharBuffer;
import java.util.ArrayList;
import java.util.List;


@Beta
@GwtIncompatible
public final class CharStreams {

    private static final int DEFAULT_BUF_SIZE = 0x800;

  
  static CharBuffer createBuffer() {
    return CharBuffer.allocate(DEFAULT_BUF_SIZE);
  }

  private CharStreams() {}

  
  @CanIgnoreReturnValue
  public static long copy(Readable from, Appendable to) throws IOException {
            if (from instanceof Reader) {
            if (to instanceof StringBuilder) {
        return copyReaderToBuilder((Reader) from, (StringBuilder) to);
      } else {
        return copyReaderToWriter((Reader) from, asWriter(to));
      }
    } else {
      checkNotNull(from);
      checkNotNull(to);
      long total = 0;
      CharBuffer buf = createBuffer();
      while (from.read(buf) != -1) {
        buf.flip();
        to.append(buf);
        total += buf.remaining();
        buf.clear();
      }
      return total;
    }
  }

      
  
  @CanIgnoreReturnValue
  static long copyReaderToBuilder(Reader from, StringBuilder to) throws IOException {
    checkNotNull(from);
    checkNotNull(to);
    char[] buf = new char[DEFAULT_BUF_SIZE];
    int nRead;
    long total = 0;
    while ((nRead = from.read(buf)) != -1) {
      to.append(buf, 0, nRead);
      total += nRead;
    }
    return total;
  }

  
  @CanIgnoreReturnValue
  static long copyReaderToWriter(Reader from, Writer to) throws IOException {
    checkNotNull(from);
    checkNotNull(to);
    char[] buf = new char[DEFAULT_BUF_SIZE];
    int nRead;
    long total = 0;
    while ((nRead = from.read(buf)) != -1) {
      to.write(buf, 0, nRead);
      total += nRead;
    }
    return total;
  }

  
  public static String toString(Readable r) throws IOException {
    return toStringBuilder(r).toString();
  }

  
  private static StringBuilder toStringBuilder(Readable r) throws IOException {
    StringBuilder sb = new StringBuilder();
    if (r instanceof Reader) {
      copyReaderToBuilder((Reader) r, sb);
    } else {
      copy(r, sb);
    }
    return sb;
  }

  
  public static List<String> readLines(Readable r) throws IOException {
    List<String> result = new ArrayList<>();
    LineReader lineReader = new LineReader(r);
    String line;
    while ((line = lineReader.readLine()) != null) {
      result.add(line);
    }
    return result;
  }

  
  @CanIgnoreReturnValue   public static <T> T readLines(Readable readable, LineProcessor<T> processor) throws IOException {
    checkNotNull(readable);
    checkNotNull(processor);

    LineReader lineReader = new LineReader(readable);
    String line;
    while ((line = lineReader.readLine()) != null) {
      if (!processor.processLine(line)) {
        break;
      }
    }
    return processor.getResult();
  }

  
  @CanIgnoreReturnValue
  public static long exhaust(Readable readable) throws IOException {
    long total = 0;
    long read;
    CharBuffer buf = createBuffer();
    while ((read = readable.read(buf)) != -1) {
      total += read;
      buf.clear();
    }
    return total;
  }

  
  public static void skipFully(Reader reader, long n) throws IOException {
    checkNotNull(reader);
    while (n > 0) {
      long amt = reader.skip(n);
      if (amt == 0) {
        throw new EOFException();
      }
      n -= amt;
    }
  }

  
  public static Writer nullWriter() {
    return NullWriter.INSTANCE;
  }

  private static final class NullWriter extends Writer {

    private static final NullWriter INSTANCE = new NullWriter();

    @Override
    public void write(int c) {}

    @Override
    public void write(char[] cbuf) {
      checkNotNull(cbuf);
    }

    @Override
    public void write(char[] cbuf, int off, int len) {
      checkPositionIndexes(off, off + len, cbuf.length);
    }

    @Override
    public void write(String str) {
      checkNotNull(str);
    }

    @Override
    public void write(String str, int off, int len) {
      checkPositionIndexes(off, off + len, str.length());
    }

    @Override
    public Writer append(CharSequence csq) {
      checkNotNull(csq);
      return this;
    }

    @Override
    public Writer append(CharSequence csq, int start, int end) {
      checkPositionIndexes(start, end, csq.length());
      return this;
    }

    @Override
    public Writer append(char c) {
      return this;
    }

    @Override
    public void flush() {}

    @Override
    public void close() {}

    @Override
    public String toString() {
      return "CharStreams.nullWriter()";
    }
  }

  
  public static Writer asWriter(Appendable target) {
    if (target instanceof Writer) {
      return (Writer) target;
    }
    return new AppendableWriter(target);
  }
}
