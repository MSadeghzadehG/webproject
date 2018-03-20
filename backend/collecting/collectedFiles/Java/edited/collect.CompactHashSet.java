

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.collect.CollectPreconditions.checkRemove;
import static com.google.common.collect.Hashing.smearedHash;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.base.Objects;
import com.google.common.base.Preconditions;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.AbstractSet;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.ConcurrentModificationException;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.Spliterator;
import java.util.Spliterators;
import java.util.function.Consumer;
import org.checkerframework.checker.nullness.compatqual.MonotonicNonNullDecl;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtIncompatible class CompactHashSet<E> extends AbstractSet<E> implements Serializable {
  
  
  public static <E> CompactHashSet<E> create() {
    return new CompactHashSet<E>();
  }

  
  public static <E> CompactHashSet<E> create(Collection<? extends E> collection) {
    CompactHashSet<E> set = createWithExpectedSize(collection.size());
    set.addAll(collection);
    return set;
  }

  
  public static <E> CompactHashSet<E> create(E... elements) {
    CompactHashSet<E> set = createWithExpectedSize(elements.length);
    Collections.addAll(set, elements);
    return set;
  }

  
  public static <E> CompactHashSet<E> createWithExpectedSize(int expectedSize) {
    return new CompactHashSet<E>(expectedSize);
  }

  private static final int MAXIMUM_CAPACITY = 1 << 30;

    private static final float DEFAULT_LOAD_FACTOR = 1.0f;

  
  private static final long NEXT_MASK = (1L << 32) - 1;

  
  private static final long HASH_MASK = ~NEXT_MASK;

    private static final int DEFAULT_SIZE = 3;

  static final int UNSET = -1;

  
  @MonotonicNonNullDecl private transient int[] table;

  
  @MonotonicNonNullDecl private transient long[] entries;

  
  @MonotonicNonNullDecl transient Object[] elements;

  
  transient float loadFactor;

  
  transient int modCount;

  
  private transient int threshold;

  
  private transient int size;

  
  CompactHashSet() {
    init(DEFAULT_SIZE, DEFAULT_LOAD_FACTOR);
  }

  
  CompactHashSet(int expectedSize) {
    init(expectedSize, DEFAULT_LOAD_FACTOR);
  }

  
  void init(int expectedSize, float loadFactor) {
    Preconditions.checkArgument(expectedSize >= 0, "Initial capacity must be non-negative");
    Preconditions.checkArgument(loadFactor > 0, "Illegal load factor");
    int buckets = Hashing.closedTableSize(expectedSize, loadFactor);
    this.table = newTable(buckets);
    this.loadFactor = loadFactor;
    this.elements = new Object[expectedSize];
    this.entries = newEntries(expectedSize);
    this.threshold = Math.max(1, (int) (buckets * loadFactor));
  }

  private static int[] newTable(int size) {
    int[] array = new int[size];
    Arrays.fill(array, UNSET);
    return array;
  }

  private static long[] newEntries(int size) {
    long[] array = new long[size];
    Arrays.fill(array, UNSET);
    return array;
  }

  private static int getHash(long entry) {
    return (int) (entry >>> 32);
  }

  
  private static int getNext(long entry) {
    return (int) entry;
  }

  
  private static long swapNext(long entry, int newNext) {
    return (HASH_MASK & entry) | (NEXT_MASK & newNext);
  }

  private int hashTableMask() {
    return table.length - 1;
  }

  @CanIgnoreReturnValue
  @Override
  public boolean add(@NullableDecl E object) {
    long[] entries = this.entries;
    Object[] elements = this.elements;
    int hash = smearedHash(object);
    int tableIndex = hash & hashTableMask();
    int newEntryIndex = this.size;     int next = table[tableIndex];
    if (next == UNSET) {       table[tableIndex] = newEntryIndex;
    } else {
      int last;
      long entry;
      do {
        last = next;
        entry = entries[next];
        if (getHash(entry) == hash && Objects.equal(object, elements[next])) {
          return false;
        }
        next = getNext(entry);
      } while (next != UNSET);
      entries[last] = swapNext(entry, newEntryIndex);
    }
    if (newEntryIndex == Integer.MAX_VALUE) {
      throw new IllegalStateException("Cannot contain more than Integer.MAX_VALUE elements!");
    }
    int newSize = newEntryIndex + 1;
    resizeMeMaybe(newSize);
    insertEntry(newEntryIndex, object, hash);
    this.size = newSize;
    if (newEntryIndex >= threshold) {
      resizeTable(2 * table.length);
    }
    modCount++;
    return true;
  }

  
  void insertEntry(int entryIndex, E object, int hash) {
    this.entries[entryIndex] = ((long) hash << 32) | (NEXT_MASK & UNSET);
    this.elements[entryIndex] = object;
  }

  
  private void resizeMeMaybe(int newSize) {
    int entriesSize = entries.length;
    if (newSize > entriesSize) {
      int newCapacity = entriesSize + Math.max(1, entriesSize >>> 1);
      if (newCapacity < 0) {
        newCapacity = Integer.MAX_VALUE;
      }
      if (newCapacity != entriesSize) {
        resizeEntries(newCapacity);
      }
    }
  }

  
  void resizeEntries(int newCapacity) {
    this.elements = Arrays.copyOf(elements, newCapacity);
    long[] entries = this.entries;
    int oldSize = entries.length;
    entries = Arrays.copyOf(entries, newCapacity);
    if (newCapacity > oldSize) {
      Arrays.fill(entries, oldSize, newCapacity, UNSET);
    }
    this.entries = entries;
  }

  private void resizeTable(int newCapacity) {     int[] oldTable = table;
    int oldCapacity = oldTable.length;
    if (oldCapacity >= MAXIMUM_CAPACITY) {
      threshold = Integer.MAX_VALUE;
      return;
    }
    int newThreshold = 1 + (int) (newCapacity * loadFactor);
    int[] newTable = newTable(newCapacity);
    long[] entries = this.entries;

    int mask = newTable.length - 1;
    for (int i = 0; i < size; i++) {
      long oldEntry = entries[i];
      int hash = getHash(oldEntry);
      int tableIndex = hash & mask;
      int next = newTable[tableIndex];
      newTable[tableIndex] = i;
      entries[i] = ((long) hash << 32) | (NEXT_MASK & next);
    }

    this.threshold = newThreshold;
    this.table = newTable;
  }

  @Override
  public boolean contains(@NullableDecl Object object) {
    int hash = smearedHash(object);
    int next = table[hash & hashTableMask()];
    while (next != UNSET) {
      long entry = entries[next];
      if (getHash(entry) == hash && Objects.equal(object, elements[next])) {
        return true;
      }
      next = getNext(entry);
    }
    return false;
  }

  @CanIgnoreReturnValue
  @Override
  public boolean remove(@NullableDecl Object object) {
    return remove(object, smearedHash(object));
  }

  @CanIgnoreReturnValue
  private boolean remove(Object object, int hash) {
    int tableIndex = hash & hashTableMask();
    int next = table[tableIndex];
    if (next == UNSET) {
      return false;
    }
    int last = UNSET;
    do {
      if (getHash(entries[next]) == hash && Objects.equal(object, elements[next])) {
        if (last == UNSET) {
                    table[tableIndex] = getNext(entries[next]);
        } else {
                    entries[last] = swapNext(entries[last], getNext(entries[next]));
        }

        moveEntry(next);
        size--;
        modCount++;
        return true;
      }
      last = next;
      next = getNext(entries[next]);
    } while (next != UNSET);
    return false;
  }

  
  void moveEntry(int dstIndex) {
    int srcIndex = size() - 1;
    if (dstIndex < srcIndex) {
            elements[dstIndex] = elements[srcIndex];
      elements[srcIndex] = null;

            long lastEntry = entries[srcIndex];
      entries[dstIndex] = lastEntry;
      entries[srcIndex] = UNSET;

                  int tableIndex = getHash(lastEntry) & hashTableMask();
      int lastNext = table[tableIndex];
      if (lastNext == srcIndex) {
                table[tableIndex] = dstIndex;
      } else {
                int previous;
        long entry;
        do {
          previous = lastNext;
          lastNext = getNext(entry = entries[lastNext]);
        } while (lastNext != srcIndex);
                entries[previous] = swapNext(entry, dstIndex);
      }
    } else {
      elements[dstIndex] = null;
      entries[dstIndex] = UNSET;
    }
  }

  int firstEntryIndex() {
    return isEmpty() ? -1 : 0;
  }

  int getSuccessor(int entryIndex) {
    return (entryIndex + 1 < size) ? entryIndex + 1 : -1;
  }

  
  int adjustAfterRemove(int indexBeforeRemove, @SuppressWarnings("unused") int indexRemoved) {
    return indexBeforeRemove - 1;
  }

  @Override
  public Iterator<E> iterator() {
    return new Iterator<E>() {
      int expectedModCount = modCount;
      int index = firstEntryIndex();
      int indexToRemove = -1;

      @Override
      public boolean hasNext() {
        return index >= 0;
      }

      @Override
      @SuppressWarnings("unchecked")
      public E next() {
        checkForConcurrentModification();
        if (!hasNext()) {
          throw new NoSuchElementException();
        }
        indexToRemove = index;
        E result = (E) elements[index];
        index = getSuccessor(index);
        return result;
      }

      @Override
      public void remove() {
        checkForConcurrentModification();
        checkRemove(indexToRemove >= 0);
        expectedModCount++;
        CompactHashSet.this.remove(elements[indexToRemove], getHash(entries[indexToRemove]));
        index = adjustAfterRemove(index, indexToRemove);
        indexToRemove = -1;
      }

      private void checkForConcurrentModification() {
        if (modCount != expectedModCount) {
          throw new ConcurrentModificationException();
        }
      }
    };
  }

  @Override
  public Spliterator<E> spliterator() {
    return Spliterators.spliterator(elements, 0, size, Spliterator.DISTINCT | Spliterator.ORDERED);
  }

  @Override
  public void forEach(Consumer<? super E> action) {
    checkNotNull(action);
    for (int i = 0; i < size; i++) {
      action.accept((E) elements[i]);
    }
  }

  @Override
  public int size() {
    return size;
  }

  @Override
  public boolean isEmpty() {
    return size == 0;
  }

  @Override
  public Object[] toArray() {
    return Arrays.copyOf(elements, size);
  }

  @CanIgnoreReturnValue
  @Override
  public <T> T[] toArray(T[] a) {
    return ObjectArrays.toArrayImpl(elements, 0, size, a);
  }

  
  public void trimToSize() {
    int size = this.size;
    if (size < entries.length) {
      resizeEntries(size);
    }
                    int minimumTableSize = Math.max(1, Integer.highestOneBit((int) (size / loadFactor)));
    if (minimumTableSize < MAXIMUM_CAPACITY) {
      double load = (double) size / minimumTableSize;
      if (load > loadFactor) {
        minimumTableSize <<= 1;       }
    }

    if (minimumTableSize < table.length) {
      resizeTable(minimumTableSize);
    }
  }

  @Override
  public void clear() {
    modCount++;
    Arrays.fill(elements, 0, size, null);
    Arrays.fill(table, UNSET);
    Arrays.fill(entries, UNSET);
    this.size = 0;
  }

  
  private void writeObject(ObjectOutputStream stream) throws IOException {
    stream.defaultWriteObject();
    stream.writeInt(size);
    for (E e : this) {
      stream.writeObject(e);
    }
  }

  @SuppressWarnings("unchecked")
  private void readObject(ObjectInputStream stream) throws IOException, ClassNotFoundException {
    stream.defaultReadObject();
    init(DEFAULT_SIZE, DEFAULT_LOAD_FACTOR);
    int elementCount = stream.readInt();
    for (int i = elementCount; --i >= 0; ) {
      E element = (E) stream.readObject();
      add(element);
    }
  }
}
