

package com.google.common.collect;

import java.io.Serializable;
import java.util.Collection;
import java.util.Map;
import java.util.Set;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;

public class SynchronizedTableTest extends AbstractTableTest {
  private static final class TestTable<R, C, V> implements Table<R, C, V>, Serializable {
    final Table<R, C, V> delegate = HashBasedTable.create();
    public final Object mutex = new Integer(1); 
    @Override
    public String toString() {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.toString();
    }

    @Override
    public boolean equals(@NullableDecl Object o) {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.equals(o);
    }

    @Override
    public int hashCode() {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.hashCode();
    }

    @Override
    public int size() {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.size();
    }

    @Override
    public boolean isEmpty() {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.isEmpty();
    }

    @Override
    public boolean containsValue(@NullableDecl Object value) {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.containsValue(value);
    }

    @Override
    public void clear() {
      assertTrue(Thread.holdsLock(mutex));
      delegate.clear();
    }

    @Override
    public Collection<V> values() {
      assertTrue(Thread.holdsLock(mutex));
      
      return delegate.values();
    }

    @Override
    public Set<Cell<R, C, V>> cellSet() {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.cellSet();
    }

    @Override
    public Map<R, V> column(C columnKey) {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.column(columnKey);
    }

    @Override
    public Set<C> columnKeySet() {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.columnKeySet();
    }

    @Override
    public Map<C, Map<R, V>> columnMap() {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.columnMap();
    }

    @Override
    public boolean contains(Object rowKey, Object columnKey) {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.contains(rowKey, columnKey);
    }

    @Override
    public boolean containsColumn(Object columnKey) {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.containsColumn(columnKey);
    }

    @Override
    public boolean containsRow(Object rowKey) {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.containsRow(rowKey);
    }

    @Override
    public V get(Object rowKey, Object columnKey) {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.get(rowKey, columnKey);
    }

    @Override
    public V put(R rowKey, C columnKey, V value) {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.put(rowKey, columnKey, value);
    }

    @Override
    public void putAll(Table<? extends R, ? extends C, ? extends V> table) {
      assertTrue(Thread.holdsLock(mutex));
      delegate.putAll(table);
    }

    @Override
    public V remove(Object rowKey, Object columnKey) {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.remove(rowKey, columnKey);
    }

    @Override
    public Map<C, V> row(R rowKey) {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.row(rowKey);
    }

    @Override
    public Set<R> rowKeySet() {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.rowKeySet();
    }

    @Override
    public Map<R, Map<C, V>> rowMap() {
      assertTrue(Thread.holdsLock(mutex));
      return delegate.rowMap();
    }

    private static final long serialVersionUID = 0;
  }

  @Override
  protected Table<String, Integer, Character> create(Object... data) {
    TestTable<String, Integer, Character> table = new TestTable<>();
    Table<String, Integer, Character> synced = Synchronized.table(table, table.mutex);
    populate(synced, data);
    return synced;
  }
}
