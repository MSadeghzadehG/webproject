

package com.google.common.collect.testing.google;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.Multimap;
import com.google.common.collect.testing.SampleElements;
import com.google.common.collect.testing.TestContainerGenerator;
import java.util.Collection;
import java.util.Map.Entry;


@GwtCompatible
public interface TestMultimapGenerator<K, V, M extends Multimap<K, V>>
    extends TestContainerGenerator<M, Entry<K, V>> {

  K[] createKeyArray(int length);

  V[] createValueArray(int length);

  SampleElements<K> sampleKeys();

  SampleElements<V> sampleValues();

  Collection<V> createCollection(Iterable<? extends V> values);
}
