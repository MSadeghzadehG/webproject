

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.Map;


@GwtCompatible
public interface TestMapGenerator<K, V> extends TestContainerGenerator<Map<K, V>, Map.Entry<K, V>> {
  K[] createKeyArray(int length);

  V[] createValueArray(int length);
}
