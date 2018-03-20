

package com.google.common.graph;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.collect.UnmodifiableIterator;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.AbstractSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


class MapIteratorCache<K, V> {
  private final Map<K, V> backingMap;

        @NullableDecl private transient Entry<K, V> entrySetCache;

  MapIteratorCache(Map<K, V> backingMap) {
    this.backingMap = checkNotNull(backingMap);
  }

  @CanIgnoreReturnValue
  public V put(@NullableDecl K key, @NullableDecl V value) {
    clearCache();
    return backingMap.put(key, value);
  }

  @CanIgnoreReturnValue
  public V remove(@NullableDecl Object key) {
    clearCache();
    return backingMap.remove(key);
  }

  public void clear() {
    clearCache();
    backingMap.clear();
  }

  public V get(@NullableDecl Object key) {
    V value = getIfCached(key);
    return (value != null) ? value : getWithoutCaching(key);
  }

  public final V getWithoutCaching(@NullableDecl Object key) {
    return backingMap.get(key);
  }

  public final boolean containsKey(@NullableDecl Object key) {
    return getIfCached(key) != null || backingMap.containsKey(key);
  }

  public final Set<K> unmodifiableKeySet() {
    return new AbstractSet<K>() {
      @Override
      public UnmodifiableIterator<K> iterator() {
        final Iterator<Entry<K, V>> entryIterator = backingMap.entrySet().iterator();

        return new UnmodifiableIterator<K>() {
          @Override
          public boolean hasNext() {
            return entryIterator.hasNext();
          }

          @Override
          public K next() {
            Entry<K, V> entry = entryIterator.next();             entrySetCache = entry;
            return entry.getKey();
          }
        };
      }

      @Override
      public int size() {
        return backingMap.size();
      }

      @Override
      public boolean contains(@NullableDecl Object key) {
        return containsKey(key);
      }
    };
  }

  
  protected V getIfCached(@NullableDecl Object key) {
    Entry<K, V> entry = entrySetCache; 
        if (entry != null && entry.getKey() == key) {
      return entry.getValue();
    }
    return null;
  }

  protected void clearCache() {
    entrySetCache = null;
  }
}
