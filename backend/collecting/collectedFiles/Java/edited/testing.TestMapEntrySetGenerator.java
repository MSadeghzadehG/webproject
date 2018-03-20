

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;


@GwtCompatible
public abstract class TestMapEntrySetGenerator<K, V> implements TestSetGenerator<Map.Entry<K, V>> {
  private final SampleElements<K> keys;
  private final SampleElements<V> values;

  protected TestMapEntrySetGenerator(SampleElements<K> keys, SampleElements<V> values) {
    this.keys = keys;
    this.values = values;
  }

  @Override
  public SampleElements<Entry<K, V>> samples() {
    return SampleElements.mapEntries(keys, values);
  }

  @Override
  public Set<Entry<K, V>> create(Object... elements) {
    Entry<K, V>[] entries = createArray(elements.length);
    System.arraycopy(elements, 0, entries, 0, elements.length);
    return createFromEntries(entries);
  }

  public abstract Set<Entry<K, V>> createFromEntries(Entry<K, V>[] entries);

  @Override
  @SuppressWarnings("unchecked")   public Entry<K, V>[] createArray(int length) {
    return new Entry[length];
  }

  
  @Override
  public List<Entry<K, V>> order(List<Entry<K, V>> insertionOrder) {
    return insertionOrder;
  }
}
