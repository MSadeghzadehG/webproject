

package com.google.zxing.pdf417.decoder.ec;

import com.google.zxing.common.reedsolomon.ReedSolomonTestCase;
import org.junit.Assert;

import java.util.BitSet;
import java.util.Random;


abstract class AbstractErrorCorrectionTestCase extends Assert {

  static void corrupt(int[] received, int howMany, Random random) {
    ReedSolomonTestCase.corrupt(received, howMany, random, 929);
  }

  static int[] erase(int[] received, int howMany, Random random) {
    BitSet erased = new BitSet(received.length);
    int[] erasures = new int[howMany];
    int erasureOffset = 0;
    for (int j = 0; j < howMany; j++) {
      int location = random.nextInt(received.length);
      if (erased.get(location)) {
        j--;
      } else {
        erased.set(location);
        received[location] = 0;
        erasures[erasureOffset++] = location;
      }
    }
    return erasures;
  }

  static Random getRandom() {
    return new Random(0xDEADBEEF);
  }

}