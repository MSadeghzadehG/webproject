

package com.google.common.collect.testing.google;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.BiMap;
import com.google.common.collect.testing.TestContainerGenerator;
import java.util.Map.Entry;


@GwtCompatible
public interface TestBiMapGenerator<K, V> extends TestContainerGenerator<BiMap<K, V>, Entry<K, V>> {
  K[] createKeyArray(int length);

  V[] createValueArray(int length);
}
