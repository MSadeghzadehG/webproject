

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtIncompatible;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Spliterator;
import java.util.Spliterators;
import java.util.function.Consumer;
import org.checkerframework.checker.nullness.compatqual.MonotonicNonNullDecl;


@GwtIncompatible class CompactLinkedHashSet<E> extends CompactHashSet<E> {

  
  public static <E> CompactLinkedHashSet<E> create() {
    return new CompactLinkedHashSet<E>();
  }

  
  public static <E> CompactLinkedHashSet<E> create(Collection<? extends E> collection) {
    CompactLinkedHashSet<E> set = createWithExpectedSize(collection.size());
    set.addAll(collection);
    return set;
  }

  
  public static <E> CompactLinkedHashSet<E> create(E... elements) {
    CompactLinkedHashSet<E> set = createWithExpectedSize(elements.length);
    Collections.addAll(set, elements);
    return set;
  }

  
  public static <E> CompactLinkedHashSet<E> createWithExpectedSize(int expectedSize) {
    return new CompactLinkedHashSet<E>(expectedSize);
  }

  private static final int ENDPOINT = -2;

      
  
  @MonotonicNonNullDecl private transient int[] predecessor;

  
  @MonotonicNonNullDecl private transient int[] successor;

  private transient int firstEntry;
  private transient int lastEntry;

  CompactLinkedHashSet() {
    super();
  }

  CompactLinkedHashSet(int expectedSize) {
    super(expectedSize);
  }

  @Override
  void init(int expectedSize, float loadFactor) {
    super.init(expectedSize, loadFactor);
    this.predecessor = new int[expectedSize];
    this.successor = new int[expectedSize];

    Arrays.fill(predecessor, UNSET);
    Arrays.fill(successor, UNSET);
    firstEntry = ENDPOINT;
    lastEntry = ENDPOINT;
  }

  private void succeeds(int pred, int succ) {
    if (pred == ENDPOINT) {
      firstEntry = succ;
    } else {
      successor[pred] = succ;
    }

    if (succ == ENDPOINT) {
      lastEntry = pred;
    } else {
      predecessor[succ] = pred;
    }
  }

  @Override
  void insertEntry(int entryIndex, E object, int hash) {
    super.insertEntry(entryIndex, object, hash);
    succeeds(lastEntry, entryIndex);
    succeeds(entryIndex, ENDPOINT);
  }

  @Override
  void moveEntry(int dstIndex) {
    int srcIndex = size() - 1;
    super.moveEntry(dstIndex);

    succeeds(predecessor[dstIndex], successor[dstIndex]);
    if (srcIndex != dstIndex) {
      succeeds(predecessor[srcIndex], dstIndex);
      succeeds(dstIndex, successor[srcIndex]);
    }
    predecessor[srcIndex] = UNSET;
    successor[srcIndex] = UNSET;
  }

  @Override
  public void clear() {
    super.clear();
    firstEntry = ENDPOINT;
    lastEntry = ENDPOINT;
    Arrays.fill(predecessor, UNSET);
    Arrays.fill(successor, UNSET);
  }

  @Override
  void resizeEntries(int newCapacity) {
    super.resizeEntries(newCapacity);
    int oldCapacity = predecessor.length;
    predecessor = Arrays.copyOf(predecessor, newCapacity);
    successor = Arrays.copyOf(successor, newCapacity);

    if (oldCapacity < newCapacity) {
      Arrays.fill(predecessor, oldCapacity, newCapacity, UNSET);
      Arrays.fill(successor, oldCapacity, newCapacity, UNSET);
    }
  }

  @Override
  public Object[] toArray() {
    return ObjectArrays.toArrayImpl(this);
  }

  @Override
  public <T> T[] toArray(T[] a) {
    return ObjectArrays.toArrayImpl(this, a);
  }

  @Override
  int firstEntryIndex() {
    return firstEntry;
  }

  @Override
  int adjustAfterRemove(int indexBeforeRemove, int indexRemoved) {
    return (indexBeforeRemove == size()) ? indexRemoved : indexBeforeRemove;
  }

  @Override
  int getSuccessor(int entryIndex) {
    return successor[entryIndex];
  }

  @Override public Spliterator<E> spliterator() {
    return Spliterators.spliterator(this, Spliterator.ORDERED | Spliterator.DISTINCT);
  }

  @Override public void forEach(Consumer<? super E> action) {
    checkNotNull(action);
    for (int i = firstEntry; i != ENDPOINT; i = successor[i]) {
      action.accept((E) elements[i]);
    }
  }
}
