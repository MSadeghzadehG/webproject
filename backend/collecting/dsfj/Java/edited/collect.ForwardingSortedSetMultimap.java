

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import java.util.Comparator;
import java.util.SortedSet;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
public abstract class ForwardingSortedSetMultimap<K, V> extends ForwardingSetMultimap<K, V>
    implements SortedSetMultimap<K, V> {

  
  protected ForwardingSortedSetMultimap() {}

  @Override
  protected abstract SortedSetMultimap<K, V> delegate();

  @Override
  public SortedSet<V> get(@NullableDecl K key) {
    return delegate().get(key);
  }

  @Override
  public SortedSet<V> removeAll(@NullableDecl Object key) {
    return delegate().removeAll(key);
  }

  @Override
  public SortedSet<V> replaceValues(K key, Iterable<? extends V> values) {
    return delegate().replaceValues(key, values);
  }

  @Override
  public Comparator<? super V> valueComparator() {
    return delegate().valueComparator();
  }
}
