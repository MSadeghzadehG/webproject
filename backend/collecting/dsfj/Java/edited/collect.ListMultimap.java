

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
public interface ListMultimap<K, V> extends Multimap<K, V> {
  
  @Override
  List<V> get(@NullableDecl K key);

  
  @CanIgnoreReturnValue
  @Override
  List<V> removeAll(@NullableDecl Object key);

  
  @CanIgnoreReturnValue
  @Override
  List<V> replaceValues(K key, Iterable<? extends V> values);

  
  @Override
  Map<K, Collection<V>> asMap();

  
  @Override
  boolean equals(@NullableDecl Object obj);
}
