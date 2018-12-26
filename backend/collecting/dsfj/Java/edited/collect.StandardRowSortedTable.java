

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtCompatible;
import com.google.common.base.Supplier;
import com.google.j2objc.annotations.WeakOuter;
import java.util.Comparator;
import java.util.Map;
import java.util.Set;
import java.util.SortedMap;
import java.util.SortedSet;


@GwtCompatible
class StandardRowSortedTable<R, C, V> extends StandardTable<R, C, V>
    implements RowSortedTable<R, C, V> {
  

  StandardRowSortedTable(
      SortedMap<R, Map<C, V>> backingMap, Supplier<? extends Map<C, V>> factory) {
    super(backingMap, factory);
  }

  private SortedMap<R, Map<C, V>> sortedBackingMap() {
    return (SortedMap<R, Map<C, V>>) backingMap;
  }

  
  @Override
  public SortedSet<R> rowKeySet() {
    return (SortedSet<R>) rowMap().keySet();
  }

  
  @Override
  public SortedMap<R, Map<C, V>> rowMap() {
    return (SortedMap<R, Map<C, V>>) super.rowMap();
  }

  @Override
  SortedMap<R, Map<C, V>> createRowMap() {
    return new RowSortedMap();
  }

  @WeakOuter
  private class RowSortedMap extends RowMap implements SortedMap<R, Map<C, V>> {
    @Override
    public SortedSet<R> keySet() {
      return (SortedSet<R>) super.keySet();
    }

    @Override
    SortedSet<R> createKeySet() {
      return new Maps.SortedKeySet<>(this);
    }

    @Override
    public Comparator<? super R> comparator() {
      return sortedBackingMap().comparator();
    }

    @Override
    public R firstKey() {
      return sortedBackingMap().firstKey();
    }

    @Override
    public R lastKey() {
      return sortedBackingMap().lastKey();
    }

    @Override
    public SortedMap<R, Map<C, V>> headMap(R toKey) {
      checkNotNull(toKey);
      return new StandardRowSortedTable<R, C, V>(sortedBackingMap().headMap(toKey), factory)
          .rowMap();
    }

    @Override
    public SortedMap<R, Map<C, V>> subMap(R fromKey, R toKey) {
      checkNotNull(fromKey);
      checkNotNull(toKey);
      return new StandardRowSortedTable<R, C, V>(sortedBackingMap().subMap(fromKey, toKey), factory)
          .rowMap();
    }

    @Override
    public SortedMap<R, Map<C, V>> tailMap(R fromKey) {
      checkNotNull(fromKey);
      return new StandardRowSortedTable<R, C, V>(sortedBackingMap().tailMap(fromKey), factory)
          .rowMap();
    }
  }

  private static final long serialVersionUID = 0;
}
