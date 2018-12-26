

package com.google.common.io;

import java.io.Closeable;
import java.io.Flushable;
import java.io.IOException;
import java.io.Writer;


public class AppendableWriterTest extends IoTestCase {

  
  private static class SpyAppendable implements Appendable, Flushable, Closeable {
    boolean flushed;
    boolean closed;
    StringBuilder result = new StringBuilder();

    @Override
    public Appendable append(CharSequence csq) {
      result.append(csq);
      return this;
    }

    @Override
    public Appendable append(char c) {
      result.append(c);
      return this;
    }

    @Override
    public Appendable append(CharSequence csq, int start, int end) {
      result.append(csq, start, end);
      return this;
    }

    @Override
    public void flush() {
      flushed = true;
    }

    @Override
    public void close() {
      closed = true;
    }
  }

  public void testWriteMethods() throws IOException {
    StringBuilder builder = new StringBuilder();
    Writer writer = new AppendableWriter(builder);

    writer.write("Hello".toCharArray());
    writer.write(',');
    writer.write(0xBEEF0020);     writer.write("Wo");
    writer.write("Whirled".toCharArray(), 3, 2);
    writer.write("Mad! Mad, I say", 2, 2);

    assertEquals("Hello, World!", builder.toString());
  }

  public void testAppendMethods() throws IOException {
    StringBuilder builder = new StringBuilder();
    Writer writer = new AppendableWriter(builder);

    writer.append("Hello,");
    writer.append(' ');
    writer.append("The World Wide Web", 4, 9);
    writer.append("!");

    assertEquals("Hello, World!", builder.toString());
  }

  public void testCloseFlush() throws IOException {
    SpyAppendable spy = new SpyAppendable();
    Writer writer = new AppendableWriter(spy);

    writer.write("Hello");
    assertFalse(spy.flushed);
    assertFalse(spy.closed);

    writer.flush();
    assertTrue(spy.flushed);
    assertFalse(spy.closed);

    writer.close();
    assertTrue(spy.flushed);
    assertTrue(spy.closed);
  }

  public void testCloseIsFinal() throws IOException {
    StringBuilder builder = new StringBuilder();
    Writer writer = new AppendableWriter(builder);

    writer.write("Hi");
    writer.close();

    try {
      writer.write(" Greg");
      fail("Should have thrown IOException due to writer already closed");
    } catch (IOException expected) {
    }

    try {
      writer.flush();
      fail("Should have thrown IOException due to writer already closed");
    } catch (IOException expected) {
    }

        writer.close();
  }
}
