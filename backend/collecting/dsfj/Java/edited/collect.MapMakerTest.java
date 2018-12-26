

package com.google.common.collect;

import static com.google.common.util.concurrent.Uninterruptibles.awaitUninterruptibly;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.base.Function;
import com.google.common.testing.NullPointerTester;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import junit.framework.TestCase;


@GwtCompatible(emulated = true)
public class MapMakerTest extends TestCase {

  @GwtIncompatible   public void testNullParameters() throws Exception {
    NullPointerTester tester = new NullPointerTester();
    tester.testAllPublicInstanceMethods(new MapMaker());
  }

  @GwtIncompatible   static final class DelayingIdentityLoader<T> implements Function<T, T> {
    private final CountDownLatch delayLatch;

    DelayingIdentityLoader(CountDownLatch delayLatch) {
      this.delayLatch = delayLatch;
    }

    @Override
    public T apply(T key) {
      awaitUninterruptibly(delayLatch);
      return key;
    }
  }

  

  
  public static class MakerTest extends TestCase {
    public void testInitialCapacity_negative() {
      MapMaker maker = new MapMaker();
      try {
        maker.initialCapacity(-1);
        fail();
      } catch (IllegalArgumentException expected) {
      }
    }

        public void xtestInitialCapacity_setTwice() {
      MapMaker maker = new MapMaker().initialCapacity(16);
      try {
                maker.initialCapacity(16);
        fail();
      } catch (IllegalArgumentException expected) {
      }
    }

    public void testReturnsPlainConcurrentHashMapWhenPossible() {
      Map<?, ?> map = new MapMaker().initialCapacity(5).makeMap();
      assertTrue(map instanceof ConcurrentHashMap);
    }
  }
}
