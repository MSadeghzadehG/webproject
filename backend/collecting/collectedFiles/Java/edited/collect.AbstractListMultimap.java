

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
abstract class AbstractListMultimap<K, V> extends AbstractMapBasedMultimap<K, V>
    implements ListMultimap<K, V> {
  
  protected AbstractListMultimap(Map<K, Collection<V>> map) {
    super(map);
  }

  @Override
  abstract List<V> createCollection();

  @Override
  List<V> createUnmodifiableEmptyCollection() {
    return Collections.emptyList();
  }

  @Override
  <E> Collection<E> unmodifiableCollectionSubclass(Collection<E> collection) {
    return Collections.unmodifiableList((List<E>) collection);
  }

  @Override
  Collection<V> wrapCollection(K key, Collection<V> collection) {
    return wrapList(key, (List<V>) collection, null);
  }

  
  
  @Override
  public List<V> get(@NullableDecl K key) {
    return (List<V>) super.get(key);
  }

  
  @CanIgnoreReturnValue
  @Override
  public List<V> removeAll(@NullableDecl Object key) {
    return (List<V>) super.removeAll(key);
  }

  
  @CanIgnoreReturnValue
  @Override
  public List<V> replaceValues(@NullableDecl K key, Iterable<? extends V> values) {
    return (List<V>) super.replaceValues(key, values);
  }

  
  @CanIgnoreReturnValue
  @Override
  public boolean put(@NullableDecl K key, @NullableDecl V value) {
    return super.put(key, value);
  }

  
  @Override
  public Map<K, Collection<V>> asMap() {
    return super.asMap();
  }

  
  @Override
  public boolean equals(@NullableDecl Object object) {
    return super.equals(object);
  }

  private static final long serialVersionUID = 6588350623831699109L;
}
