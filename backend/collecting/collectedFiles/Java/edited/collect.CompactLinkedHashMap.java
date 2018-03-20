
package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.annotations.VisibleForTesting;
import com.google.j2objc.annotations.WeakOuter;
import java.util.Arrays;
import java.util.Collection;
import java.util.Set;
import java.util.Spliterator;
import java.util.Spliterators;
import java.util.function.BiConsumer;
import java.util.function.Consumer;
import org.checkerframework.checker.nullness.compatqual.MonotonicNonNullDecl;


@GwtIncompatible class CompactLinkedHashMap<K, V> extends CompactHashMap<K, V> {
  
  
  public static <K, V> CompactLinkedHashMap<K, V> create() {
    return new CompactLinkedHashMap<>();
  }

  
  public static <K, V> CompactLinkedHashMap<K, V> createWithExpectedSize(int expectedSize) {
    return new CompactLinkedHashMap<>(expectedSize);
  }

  private static final int ENDPOINT = -2;

  
  @MonotonicNonNullDecl @VisibleForTesting transient long[] links;

  
  private transient int firstEntry;

  
  private transient int lastEntry;

  private final boolean accessOrder;

  CompactLinkedHashMap() {
    this(DEFAULT_SIZE);
  }

  CompactLinkedHashMap(int expectedSize) {
    this(expectedSize, DEFAULT_LOAD_FACTOR, false);
  }

  CompactLinkedHashMap(int expectedSize, float loadFactor, boolean accessOrder) {
    super(expectedSize, loadFactor);
    this.accessOrder = accessOrder;
  }

  @Override
  void init(int expectedSize, float loadFactor) {
    super.init(expectedSize, loadFactor);
    firstEntry = ENDPOINT;
    lastEntry = ENDPOINT;
    links = new long[expectedSize];
    Arrays.fill(links, UNSET);
  }

  private int getPredecessor(int entry) {
    return (int) (links[entry] >>> 32);
  }

  @Override
  int getSuccessor(int entry) {
    return (int) links[entry];
  }

  private void setSuccessor(int entry, int succ) {
    long succMask = (~0L) >>> 32;
    links[entry] = (links[entry] & ~succMask) | (succ & succMask);
  }

  private void setPredecessor(int entry, int pred) {
    long predMask = (~0L) << 32;
    links[entry] = (links[entry] & ~predMask) | ((long) pred << 32);
  }

  private void setSucceeds(int pred, int succ) {
    if (pred == ENDPOINT) {
      firstEntry = succ;
    } else {
      setSuccessor(pred, succ);
    }
    if (succ == ENDPOINT) {
      lastEntry = pred;
    } else {
      setPredecessor(succ, pred);
    }
  }

  @Override
  void insertEntry(int entryIndex, K key, V value, int hash) {
    super.insertEntry(entryIndex, key, value, hash);
    setSucceeds(lastEntry, entryIndex);
    setSucceeds(entryIndex, ENDPOINT);
  }

  @Override
  void accessEntry(int index) {
    if (accessOrder) {
            setSucceeds(getPredecessor(index), getSuccessor(index));
            setSucceeds(lastEntry, index);
      setSucceeds(index, ENDPOINT);
      modCount++;
    }
  }

  @Override
  void moveLastEntry(int dstIndex) {
    int srcIndex = size() - 1;
    setSucceeds(getPredecessor(dstIndex), getSuccessor(dstIndex));
    if (dstIndex < srcIndex) {
      setSucceeds(getPredecessor(srcIndex), dstIndex);
      setSucceeds(dstIndex, getSuccessor(srcIndex));
    }
    super.moveLastEntry(dstIndex);
  }

  @Override
  void resizeEntries(int newCapacity) {
    super.resizeEntries(newCapacity);
    links = Arrays.copyOf(links, newCapacity);
  }

  @Override
  int firstEntryIndex() {
    return firstEntry;
  }

  @Override
  int adjustAfterRemove(int indexBeforeRemove, int indexRemoved) {
    return (indexBeforeRemove >= size()) ? indexRemoved : indexBeforeRemove;
  }

  @Override
  public void forEach(BiConsumer<? super K, ? super V> action) {
    checkNotNull(action);
    for (int i = firstEntry; i != ENDPOINT; i = getSuccessor(i)) {
      action.accept((K) keys[i], (V) values[i]);
    }
  }

  @Override
  Set<Entry<K, V>> createEntrySet() {
    @WeakOuter
    class EntrySetImpl extends EntrySetView {
      @Override
      public Spliterator<Entry<K, V>> spliterator() {
        return Spliterators.spliterator(this, Spliterator.ORDERED | Spliterator.DISTINCT);
      }
    }
    return new EntrySetImpl();
  }

  @Override
  Set<K> createKeySet() {
    @WeakOuter
    class KeySetImpl extends KeySetView {
      @Override
      public Object[] toArray() {
        return ObjectArrays.toArrayImpl(this);
      }

      @Override
      public <T> T[] toArray(T[] a) {
        return ObjectArrays.toArrayImpl(this, a);
      }

      @Override
      public Spliterator<K> spliterator() {
        return Spliterators.spliterator(this, Spliterator.ORDERED | Spliterator.DISTINCT);
      }

      @Override
      public void forEach(Consumer<? super K> action) {
        checkNotNull(action);
        for (int i = firstEntry; i != ENDPOINT; i = getSuccessor(i)) {
          action.accept((K) keys[i]);
        }
      }
    }
    return new KeySetImpl();
  }

  @Override
  Collection<V> createValues() {
    @WeakOuter
    class ValuesImpl extends ValuesView {
      @Override
      public Object[] toArray() {
        return ObjectArrays.toArrayImpl(this);
      }

      @Override
      public <T> T[] toArray(T[] a) {
        return ObjectArrays.toArrayImpl(this, a);
      }

      @Override
      public Spliterator<V> spliterator() {
        return Spliterators.spliterator(this, Spliterator.ORDERED);
      }

      @Override
      public void forEach(Consumer<? super V> action) {
        checkNotNull(action);
        for (int i = firstEntry; i != ENDPOINT; i = getSuccessor(i)) {
          action.accept((V) values[i]);
        }
      }
    }
    return new ValuesImpl();
  }

  @Override
  public void clear() {
    super.clear();
    this.firstEntry = ENDPOINT;
    this.lastEntry = ENDPOINT;
  }
}
