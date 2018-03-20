

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import java.util.Comparator;
import java.util.Iterator;
import java.util.Set;
import java.util.SortedSet;


@GwtCompatible(emulated = true)
abstract class DescendingMultiset<E> extends ForwardingMultiset<E> implements SortedMultiset<E> {
  abstract SortedMultiset<E> forwardMultiset();

  private transient Comparator<? super E> comparator;

  @Override
  public Comparator<? super E> comparator() {
    Comparator<? super E> result = comparator;
    if (result == null) {
      return comparator = Ordering.from(forwardMultiset().comparator()).<E>reverse();
    }
    return result;
  }

  private transient SortedSet<E> elementSet;

  @Override
  public SortedSet<E> elementSet() {
    SortedSet<E> result = elementSet;
    if (result == null) {
      return elementSet = new SortedMultisets.ElementSet<E>(this);
    }
    return result;
  }

  @Override
  public Entry<E> pollFirstEntry() {
    return forwardMultiset().pollLastEntry();
  }

  @Override
  public Entry<E> pollLastEntry() {
    return forwardMultiset().pollFirstEntry();
  }

  @Override
  public SortedMultiset<E> headMultiset(E toElement, BoundType boundType) {
    return forwardMultiset().tailMultiset(toElement, boundType).descendingMultiset();
  }

  @Override
  public SortedMultiset<E> subMultiset(
      E fromElement, BoundType fromBoundType, E toElement, BoundType toBoundType) {
    return forwardMultiset()
        .subMultiset(toElement, toBoundType, fromElement, fromBoundType)
        .descendingMultiset();
  }

  @Override
  public SortedMultiset<E> tailMultiset(E fromElement, BoundType boundType) {
    return forwardMultiset().headMultiset(fromElement, boundType).descendingMultiset();
  }

  @Override
  protected Multiset<E> delegate() {
    return forwardMultiset();
  }

  @Override
  public SortedMultiset<E> descendingMultiset() {
    return forwardMultiset();
  }

  @Override
  public Entry<E> firstEntry() {
    return forwardMultiset().lastEntry();
  }

  @Override
  public Entry<E> lastEntry() {
    return forwardMultiset().firstEntry();
  }

  abstract Iterator<Entry<E>> entryIterator();

  private transient Set<Entry<E>> entrySet;

  @Override
  public Set<Entry<E>> entrySet() {
    Set<Entry<E>> result = entrySet;
    return (result == null) ? entrySet = createEntrySet() : result;
  }

  Set<Entry<E>> createEntrySet() {
    return new Multisets.EntrySet<E>() {
      @Override
      Multiset<E> multiset() {
        return DescendingMultiset.this;
      }

      @Override
      public Iterator<Entry<E>> iterator() {
        return entryIterator();
      }

      @Override
      public int size() {
        return forwardMultiset().entrySet().size();
      }
    };
  }

  @Override
  public Iterator<E> iterator() {
    return Multisets.iteratorImpl(this);
  }

  @Override
  public Object[] toArray() {
    return standardToArray();
  }

  @Override
  public <T> T[] toArray(T[] array) {
    return standardToArray(array);
  }

  @Override
  public String toString() {
    return entrySet().toString();
  }
}
