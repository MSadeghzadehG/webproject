

package com.google.common.io;

import com.google.common.collect.ImmutableList;
import java.io.FilterReader;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;
import junit.framework.TestCase;


public class MultiReaderTest extends TestCase {

  public void testOnlyOneOpen() throws Exception {
    String testString = "abcdefgh";
    final CharSource source = newCharSource(testString);
    final int[] counter = new int[1];
    CharSource reader =
        new CharSource() {
          @Override
          public Reader openStream() throws IOException {
            if (counter[0]++ != 0) {
              throw new IllegalStateException("More than one source open");
            }
            return new FilterReader(source.openStream()) {
              @Override
              public void close() throws IOException {
                super.close();
                counter[0]--;
              }
            };
          }
        };
    Reader joinedReader = CharSource.concat(reader, reader, reader).openStream();
    String result = CharStreams.toString(joinedReader);
    assertEquals(testString.length() * 3, result.length());
  }

  public void testReady() throws Exception {
    CharSource source = newCharSource("a");
    Iterable<? extends CharSource> list = ImmutableList.of(source, source);
    Reader joinedReader = CharSource.concat(list).openStream();

    assertTrue(joinedReader.ready());
    assertEquals('a', joinedReader.read());
    assertEquals('a', joinedReader.read());
    assertEquals(-1, joinedReader.read());
    assertFalse(joinedReader.ready());
  }

  public void testSimple() throws Exception {
    String testString = "abcdefgh";
    CharSource source = newCharSource(testString);
    Reader joinedReader = CharSource.concat(source, source).openStream();

    String expectedString = testString + testString;
    assertEquals(expectedString, CharStreams.toString(joinedReader));
  }

  private static CharSource newCharSource(final String text) {
    return new CharSource() {
      @Override
      public Reader openStream() {
        return new StringReader(text);
      }
    };
  }

  public void testSkip() throws Exception {
    String begin = "abcde";
    String end = "fghij";
    Reader joinedReader = CharSource.concat(newCharSource(begin), newCharSource(end)).openStream();

    String expected = begin + end;
    assertEquals(expected.charAt(0), joinedReader.read());
    CharStreams.skipFully(joinedReader, 1);
    assertEquals(expected.charAt(2), joinedReader.read());
    CharStreams.skipFully(joinedReader, 4);
    assertEquals(expected.charAt(7), joinedReader.read());
    CharStreams.skipFully(joinedReader, 1);
    assertEquals(expected.charAt(9), joinedReader.read());
    assertEquals(-1, joinedReader.read());
  }

  public void testSkipZero() throws Exception {
    CharSource source = newCharSource("a");
    Iterable<CharSource> list = ImmutableList.of(source, source);
    Reader joinedReader = CharSource.concat(list).openStream();

    assertEquals(0, joinedReader.skip(0));
    assertEquals('a', joinedReader.read());
  }
}
