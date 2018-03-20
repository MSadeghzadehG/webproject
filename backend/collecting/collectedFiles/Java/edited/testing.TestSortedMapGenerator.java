

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.Map.Entry;
import java.util.SortedMap;


@GwtCompatible
public interface TestSortedMapGenerator<K, V> extends TestMapGenerator<K, V> {
  @Override
  SortedMap<K, V> create(Object... elements);

  
  Entry<K, V> belowSamplesLesser();

  
  Entry<K, V> belowSamplesGreater();

  
  Entry<K, V> aboveSamplesLesser();

  
  Entry<K, V> aboveSamplesGreater();
}
