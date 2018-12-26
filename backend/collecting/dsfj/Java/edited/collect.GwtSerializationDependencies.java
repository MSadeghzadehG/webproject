

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import java.io.Serializable;
import java.util.Comparator;
import java.util.HashMap;
import java.util.TreeMap;


@GwtCompatible
@SuppressWarnings("serial")
final class GwtSerializationDependencies {
  private GwtSerializationDependencies() {}

  static final class ImmutableListMultimapDependencies<K, V> extends ImmutableListMultimap<K, V> {
    K key;
    V value;

    ImmutableListMultimapDependencies() {
      super(null, 0);
    }
  }

  
  
  static final class ImmutableSetMultimapDependencies<K, V> extends ImmutableSetMultimap<K, V> {
    K key;
    V value;

    ImmutableSetMultimapDependencies() {
      super(null, 0, null);
    }
  }

  
  static final class LinkedListMultimapDependencies<K, V> extends LinkedListMultimap<K, V> {
    K key;
    V value;

    LinkedListMultimapDependencies() {}
  }

  static final class HashBasedTableDependencies<R, C, V> extends HashBasedTable<R, C, V> {
    HashMap<R, HashMap<C, V>> data;

    HashBasedTableDependencies() {
      super(null, null);
    }
  }

  static final class TreeBasedTableDependencies<R, C, V> extends TreeBasedTable<R, C, V> {
    TreeMap<R, TreeMap<C, V>> data;

    TreeBasedTableDependencies() {
      super(null, null);
    }
  }

  
  static final class ImmutableTableDependencies<R, C, V> extends SingletonImmutableTable<R, C, V>
      implements Serializable {
    R rowKey;
    C columnKey;
    V value;

    ImmutableTableDependencies() {
      super(null, null, null);
    }
  }

  static final class TreeMultimapDependencies<K, V> extends TreeMultimap<K, V> {
    Comparator<? super K> keyComparator;
    Comparator<? super V> valueComparator;
    K key;
    V value;

    TreeMultimapDependencies() {
      super(null, null);
    }
  }
}
