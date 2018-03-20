

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.j2objc.annotations.WeakOuter;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import org.checkerframework.checker.nullness.compatqual.MonotonicNonNullDecl;


@GwtCompatible
final class WellBehavedMap<K, V> extends ForwardingMap<K, V> {
  private final Map<K, V> delegate;
  @MonotonicNonNullDecl private Set<Entry<K, V>> entrySet;

  private WellBehavedMap(Map<K, V> delegate) {
    this.delegate = delegate;
  }

  
  static <K, V> WellBehavedMap<K, V> wrap(Map<K, V> delegate) {
    return new WellBehavedMap<>(delegate);
  }

  @Override
  protected Map<K, V> delegate() {
    return delegate;
  }

  @Override
  public Set<Entry<K, V>> entrySet() {
    Set<Entry<K, V>> es = entrySet;
    if (es != null) {
      return es;
    }
    return entrySet = new EntrySet();
  }

  @WeakOuter
  private final class EntrySet extends Maps.EntrySet<K, V> {
    @Override
    Map<K, V> map() {
      return WellBehavedMap.this;
    }

    @Override
    public Iterator<Entry<K, V>> iterator() {
      return new TransformedIterator<K, Entry<K, V>>(keySet().iterator()) {
        @Override
        Entry<K, V> transform(final K key) {
          return new AbstractMapEntry<K, V>() {
            @Override
            public K getKey() {
              return key;
            }

            @Override
            public V getValue() {
              return get(key);
            }

            @Override
            public V setValue(V value) {
              return put(key, value);
            }
          };
        }
      };
    }
  }
}
