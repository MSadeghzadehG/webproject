

package com.google.common.io;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.io.CharStreams.createBuffer;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.io.IOException;
import java.io.Reader;
import java.nio.CharBuffer;
import java.util.LinkedList;
import java.util.Queue;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@Beta
@GwtIncompatible
public final class LineReader {
  private final Readable readable;
  @NullableDecl private final Reader reader;
  private final CharBuffer cbuf = createBuffer();
  private final char[] buf = cbuf.array();

  private final Queue<String> lines = new LinkedList<>();
  private final LineBuffer lineBuf =
      new LineBuffer() {
        @Override
        protected void handleLine(String line, String end) {
          lines.add(line);
        }
      };

  
  public LineReader(Readable readable) {
    this.readable = checkNotNull(readable);
    this.reader = (readable instanceof Reader) ? (Reader) readable : null;
  }

  
  @CanIgnoreReturnValue   public String readLine() throws IOException {
    while (lines.peek() == null) {
      cbuf.clear();
                  int read = (reader != null) ? reader.read(buf, 0, buf.length) : readable.read(cbuf);
      if (read == -1) {
        lineBuf.finish();
        break;
      }
      lineBuf.add(buf, 0, read);
    }
    return lines.poll();
  }
}
