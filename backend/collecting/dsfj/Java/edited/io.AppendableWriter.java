

package com.google.common.io;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtIncompatible;
import java.io.Closeable;
import java.io.Flushable;
import java.io.IOException;
import java.io.Writer;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtIncompatible
class AppendableWriter extends Writer {
  private final Appendable target;
  private boolean closed;

  
  AppendableWriter(Appendable target) {
    this.target = checkNotNull(target);
  }

  

  @Override
  public void write(char[] cbuf, int off, int len) throws IOException {
    checkNotClosed();
            target.append(new String(cbuf, off, len));
  }

  

  @Override
  public void write(int c) throws IOException {
    checkNotClosed();
    target.append((char) c);
  }

  @Override
  public void write(@NullableDecl String str) throws IOException {
    checkNotClosed();
    target.append(str);
  }

  @Override
  public void write(@NullableDecl String str, int off, int len) throws IOException {
    checkNotClosed();
        target.append(str, off, off + len);
  }

  @Override
  public void flush() throws IOException {
    checkNotClosed();
    if (target instanceof Flushable) {
      ((Flushable) target).flush();
    }
  }

  @Override
  public void close() throws IOException {
    this.closed = true;
    if (target instanceof Closeable) {
      ((Closeable) target).close();
    }
  }

  @Override
  public Writer append(char c) throws IOException {
    checkNotClosed();
    target.append(c);
    return this;
  }

  @Override
  public Writer append(@NullableDecl CharSequence charSeq) throws IOException {
    checkNotClosed();
    target.append(charSeq);
    return this;
  }

  @Override
  public Writer append(@NullableDecl CharSequence charSeq, int start, int end) throws IOException {
    checkNotClosed();
    target.append(charSeq, start, end);
    return this;
  }

  private void checkNotClosed() throws IOException {
    if (closed) {
      throw new IOException("Cannot write to a closed writer.");
    }
  }
}
