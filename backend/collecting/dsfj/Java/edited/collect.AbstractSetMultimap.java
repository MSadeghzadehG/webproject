

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.Collection;
import java.util.Collections;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
abstract class AbstractSetMultimap<K, V> extends AbstractMapBasedMultimap<K, V>
    implements SetMultimap<K, V> {
  
  protected AbstractSetMultimap(Map<K, Collection<V>> map) {
    super(map);
  }

  @Override
  abstract Set<V> createCollection();

  @Override
  Set<V> createUnmodifiableEmptyCollection() {
    return Collections.emptySet();
  }

  @Override
  <E> Collection<E> unmodifiableCollectionSubclass(Collection<E> collection) {
    return Collections.unmodifiableSet((Set<E>) collection);
  }

  @Override
  Collection<V> wrapCollection(K key, Collection<V> collection) {
    return new WrappedSet(key, (Set<V>) collection);
  }

  
  
  @Override
  public Set<V> get(@NullableDecl K key) {
    return (Set<V>) super.get(key);
  }

  
  @Override
  public Set<Entry<K, V>> entries() {
    return (Set<Entry<K, V>>) super.entries();
  }

  
  @CanIgnoreReturnValue
  @Override
  public Set<V> removeAll(@NullableDecl Object key) {
    return (Set<V>) super.removeAll(key);
  }

  
  @CanIgnoreReturnValue
  @Override
  public Set<V> replaceValues(@NullableDecl K key, Iterable<? extends V> values) {
    return (Set<V>) super.replaceValues(key, values);
  }

  
  @Override
  public Map<K, Collection<V>> asMap() {
    return super.asMap();
  }

  
  @CanIgnoreReturnValue
  @Override
  public boolean put(@NullableDecl K key, @NullableDecl V value) {
    return super.put(key, value);
  }

  
  @Override
  public boolean equals(@NullableDecl Object object) {
    return super.equals(object);
  }

  private static final long serialVersionUID = 7431625294878419160L;
}
