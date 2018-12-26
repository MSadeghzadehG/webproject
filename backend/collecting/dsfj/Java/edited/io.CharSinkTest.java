

package com.google.common.io;

import static com.google.common.io.TestOption.CLOSE_THROWS;
import static com.google.common.io.TestOption.OPEN_THROWS;
import static com.google.common.io.TestOption.READ_THROWS;
import static com.google.common.io.TestOption.WRITE_THROWS;

import com.google.common.collect.ImmutableList;
import java.io.IOException;
import java.io.StringReader;
import java.io.Writer;
import java.util.EnumSet;


public class CharSinkTest extends IoTestCase {

  private static final String STRING = ASCII + I18N;

  private TestCharSink sink;

  @Override
  public void setUp() {
    sink = new TestCharSink();
  }

  public void testOpenBufferedStream() throws IOException {
    Writer writer = sink.openBufferedStream();
    assertTrue(sink.wasStreamOpened());
    assertFalse(sink.wasStreamClosed());

    writer.write(STRING);
    writer.close();

    assertTrue(sink.wasStreamClosed());
    assertEquals(STRING, sink.getString());
  }

  public void testWrite_string() throws IOException {
    assertEquals("", sink.getString());
    sink.write(STRING);

    assertTrue(sink.wasStreamOpened() && sink.wasStreamClosed());
    assertEquals(STRING, sink.getString());
  }

  public void testWriteFrom_reader() throws IOException {
    StringReader reader = new StringReader(STRING);
    sink.writeFrom(reader);

    assertTrue(sink.wasStreamOpened() && sink.wasStreamClosed());
    assertEquals(STRING, sink.getString());
  }

  public void testWriteFromStream_doesNotCloseThatStream() throws IOException {
    TestReader in = new TestReader();
    assertFalse(in.closed());
    sink.writeFrom(in);
    assertFalse(in.closed());
  }

  public void testWriteLines_withSpecificSeparator() throws IOException {
    sink.writeLines(ImmutableList.of("foo", "bar", "baz"), "\n");
    assertEquals("foo\nbar\nbaz\n", sink.getString());
  }

  public void testWriteLines_withDefaultSeparator() throws IOException {
    sink.writeLines(ImmutableList.of("foo", "bar", "baz"));
    String separator = System.getProperty("line.separator");
    assertEquals("foo" + separator + "bar" + separator + "baz" + separator, sink.getString());
  }

  public void testClosesOnErrors_copyingFromCharSourceThatThrows() {
    for (TestOption option : EnumSet.of(OPEN_THROWS, READ_THROWS, CLOSE_THROWS)) {
      TestCharSource failSource = new TestCharSource(STRING, option);
      TestCharSink okSink = new TestCharSink();
      try {
        failSource.copyTo(okSink);
        fail();
      } catch (IOException expected) {
      }
                  assertTrue(
          "stream not closed when copying from source with option: " + option,
          !okSink.wasStreamOpened() || okSink.wasStreamClosed());
    }
  }

  public void testClosesOnErrors_whenWriteThrows() {
    TestCharSink failSink = new TestCharSink(WRITE_THROWS);
    try {
      new TestCharSource(STRING).copyTo(failSink);
      fail();
    } catch (IOException expected) {
    }
    assertTrue(failSink.wasStreamClosed());
  }

  public void testClosesOnErrors_whenWritingFromReaderThatThrows() {
    TestCharSink okSink = new TestCharSink();
    try {
      okSink.writeFrom(new TestReader(READ_THROWS));
      fail();
    } catch (IOException expected) {
    }
    assertTrue(okSink.wasStreamClosed());
  }
}
