

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.Collection;
import java.util.Comparator;
import java.util.Map;
import java.util.Set;
import java.util.SortedSet;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
public interface SortedSetMultimap<K, V> extends SetMultimap<K, V> {
  
  
  @Override
  SortedSet<V> get(@NullableDecl K key);

  
  @CanIgnoreReturnValue
  @Override
  SortedSet<V> removeAll(@NullableDecl Object key);

  
  @CanIgnoreReturnValue
  @Override
  SortedSet<V> replaceValues(K key, Iterable<? extends V> values);

  
  @Override
  Map<K, Collection<V>> asMap();

  
  Comparator<? super V> valueComparator();
}
