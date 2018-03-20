

package com.google.common.collect.testing.google;

import static com.google.common.collect.testing.features.CollectionFeature.SERIALIZABLE;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.BiMap;
import com.google.common.collect.testing.Helpers;
import com.google.common.collect.testing.features.CollectionFeature;
import com.google.common.testing.SerializableTester;
import java.io.Serializable;
import java.lang.reflect.Method;
import java.util.Collections;
import java.util.List;
import org.junit.Ignore;


@GwtCompatible(emulated = true)
@Ignore public class BiMapInverseTester<K, V> extends AbstractBiMapTester<K, V> {

  public void testInverseSame() {
    assertSame(getMap(), getMap().inverse().inverse());
  }

  @CollectionFeature.Require(SERIALIZABLE)
  public void testInverseSerialization() {
    BiMapPair<K, V> pair = new BiMapPair<>(getMap());
    BiMapPair<K, V> copy = SerializableTester.reserialize(pair);
    assertEquals(pair.forward, copy.forward);
    assertEquals(pair.backward, copy.backward);
    assertSame(copy.backward, copy.forward.inverse());
    assertSame(copy.forward, copy.backward.inverse());
  }

  private static class BiMapPair<K, V> implements Serializable {
    final BiMap<K, V> forward;
    final BiMap<V, K> backward;

    BiMapPair(BiMap<K, V> original) {
      this.forward = original;
      this.backward = original.inverse();
    }

    private static final long serialVersionUID = 0;
  }

  
  @GwtIncompatible   public static List<Method> getInverseSameAfterSerializingMethods() {
    return Collections.singletonList(getMethod("testInverseSerialization"));
  }

  @GwtIncompatible   private static Method getMethod(String methodName) {
    return Helpers.getMethod(BiMapInverseTester.class, methodName);
  }
}
