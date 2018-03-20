

package com.google.common.io;

import static com.google.common.io.SourceSinkFactory.ByteSinkFactory;
import static com.google.common.io.SourceSinkFactory.CharSinkFactory;
import static org.junit.Assert.assertArrayEquals;

import com.google.common.base.Charsets;
import com.google.common.collect.ImmutableList;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.Map.Entry;
import junit.framework.TestSuite;


@AndroidIncompatible public class ByteSinkTester extends SourceSinkTester<ByteSink, byte[], ByteSinkFactory> {

  private static final ImmutableList<Method> testMethods = getTestMethods(ByteSinkTester.class);

  static TestSuite tests(String name, ByteSinkFactory factory) {
    TestSuite suite = new TestSuite(name);
    for (Entry<String, String> entry : TEST_STRINGS.entrySet()) {
      String desc = entry.getKey();
      TestSuite stringSuite = suiteForString(name, factory, entry.getValue(), desc);
      suite.addTest(stringSuite);
    }
    return suite;
  }

  private static TestSuite suiteForString(
      String name, ByteSinkFactory factory, String string, String desc) {
    byte[] bytes = string.getBytes(Charsets.UTF_8);
    TestSuite suite = suiteForBytes(name, factory, desc, bytes);
    CharSinkFactory charSinkFactory = SourceSinkFactories.asCharSinkFactory(factory);
    suite.addTest(
        CharSinkTester.suiteForString(
            name + ".asCharSink[Charset]", charSinkFactory, string, desc));
    return suite;
  }

  private static TestSuite suiteForBytes(
      String name, ByteSinkFactory factory, String desc, byte[] bytes) {
    TestSuite suite = new TestSuite(name + " [" + desc + "]");
    for (final Method method : testMethods) {
      suite.addTest(new ByteSinkTester(factory, bytes, name, desc, method));
    }
    return suite;
  }

  private ByteSink sink;

  ByteSinkTester(
      ByteSinkFactory factory, byte[] data, String suiteName, String caseDesc, Method method) {
    super(factory, data, suiteName, caseDesc, method);
  }

  @Override
  protected void setUp() throws Exception {
    sink = factory.createSink();
  }

  public void testOpenStream() throws IOException {
    OutputStream out = sink.openStream();
    try {
      ByteStreams.copy(new ByteArrayInputStream(data), out);
    } finally {
      out.close();
    }

    assertContainsExpectedBytes();
  }

  public void testOpenBufferedStream() throws IOException {
    OutputStream out = sink.openBufferedStream();
    try {
      ByteStreams.copy(new ByteArrayInputStream(data), out);
    } finally {
      out.close();
    }

    assertContainsExpectedBytes();
  }

  public void testWrite() throws IOException {
    sink.write(data);

    assertContainsExpectedBytes();
  }

  public void testWriteFrom_inputStream() throws IOException {
    sink.writeFrom(new ByteArrayInputStream(data));

    assertContainsExpectedBytes();
  }

  private void assertContainsExpectedBytes() throws IOException {
    assertArrayEquals(expected, factory.getSinkContents());
  }
}
