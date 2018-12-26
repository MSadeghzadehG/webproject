

package com.google.common.hash;

import com.google.common.collect.ImmutableList;
import com.google.common.hash.HashTestUtils.RandomHasherAction;
import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.List;
import java.util.Random;
import junit.framework.TestCase;


public class AbstractNonStreamingHashFunctionTest extends TestCase {
  
  public void testExhaustive() {
    List<Hasher> hashers =
        ImmutableList.of(
            new StreamingVersion().newHasher(),
            new StreamingVersion().newHasher(52),
            new NonStreamingVersion().newHasher(),
            new NonStreamingVersion().newHasher(123));
    Random random = new Random(0);
    for (int i = 0; i < 200; i++) {
      RandomHasherAction.pickAtRandom(random).performAction(random, hashers);
    }
    HashCode[] codes = new HashCode[hashers.size()];
    for (int i = 0; i < hashers.size(); i++) {
      codes[i] = hashers.get(i).hash();
    }
    for (int i = 1; i < codes.length; i++) {
      assertEquals(codes[i - 1], codes[i]);
    }
  }

  public void testPutStringWithLowSurrogate() {
            assertPutString(new char[] {'p', HashTestUtils.randomLowSurrogate(new Random())});
  }

  public void testPutStringWithHighSurrogate() {
            assertPutString(new char[] {'p', HashTestUtils.randomHighSurrogate(new Random())});
  }

  public void testPutStringWithLowHighSurrogate() {
    assertPutString(
        new char[] {
          HashTestUtils.randomLowSurrogate(new Random()),
          HashTestUtils.randomHighSurrogate(new Random())
        });
  }

  public void testPutStringWithHighLowSurrogate() {
    assertPutString(
        new char[] {
          HashTestUtils.randomHighSurrogate(new Random()),
          HashTestUtils.randomLowSurrogate(new Random())
        });
  }

  private static void assertPutString(char[] chars) {
    Hasher h1 = new NonStreamingVersion().newHasher();
    Hasher h2 = new NonStreamingVersion().newHasher();
    String s = new String(chars);
        for (int i = 0; i < s.length(); i++) {
      h1.putChar(s.charAt(i));
    }
    h2.putUnencodedChars(s);
    assertEquals(h1.hash(), h2.hash());
  }

  static class StreamingVersion extends AbstractHashFunction {
    @Override
    public int bits() {
      return 32;
    }

    @Override
    public Hasher newHasher() {
      return new AbstractStreamingHasher(4, 4) {
        final ByteArrayOutputStream out = new ByteArrayOutputStream();

        @Override
        protected HashCode makeHash() {
          return HashCode.fromBytes(out.toByteArray());
        }

        @Override
        protected void process(ByteBuffer bb) {
          while (bb.hasRemaining()) {
            out.write(bb.get());
          }
        }

        @Override
        protected void processRemaining(ByteBuffer bb) {
          while (bb.hasRemaining()) {
            out.write(bb.get());
          }
        }
      };
    }
  }

  static class NonStreamingVersion extends AbstractNonStreamingHashFunction {
    @Override
    public int bits() {
      return 32;
    }

    @Override
    public HashCode hashBytes(byte[] input, int off, int len) {
      return HashCode.fromBytes(Arrays.copyOfRange(input, off, off + len));
    }
  }
}
