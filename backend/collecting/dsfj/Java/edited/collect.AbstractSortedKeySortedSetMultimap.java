

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import java.util.Collection;
import java.util.Set;
import java.util.SortedMap;
import java.util.SortedSet;


@GwtCompatible
abstract class AbstractSortedKeySortedSetMultimap<K, V> extends AbstractSortedSetMultimap<K, V> {

  AbstractSortedKeySortedSetMultimap(SortedMap<K, Collection<V>> map) {
    super(map);
  }

  @Override
  public SortedMap<K, Collection<V>> asMap() {
    return (SortedMap<K, Collection<V>>) super.asMap();
  }

  @Override
  SortedMap<K, Collection<V>> backingMap() {
    return (SortedMap<K, Collection<V>>) super.backingMap();
  }

  @Override
  public SortedSet<K> keySet() {
    return (SortedSet<K>) super.keySet();
  }

  @Override
  Set<K> createKeySet() {
    return createMaybeNavigableKeySet();
  }
}
