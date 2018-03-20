

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.Collection;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
public interface SetMultimap<K, V> extends Multimap<K, V> {
  
  @Override
  Set<V> get(@NullableDecl K key);

  
  @CanIgnoreReturnValue
  @Override
  Set<V> removeAll(@NullableDecl Object key);

  
  @CanIgnoreReturnValue
  @Override
  Set<V> replaceValues(K key, Iterable<? extends V> values);

  
  @Override
  Set<Entry<K, V>> entries();

  
  @Override
  Map<K, Collection<V>> asMap();

  
  @Override
  boolean equals(@NullableDecl Object obj);
}
