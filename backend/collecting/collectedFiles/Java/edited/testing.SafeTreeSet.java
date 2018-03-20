

package com.google.common.collect.testing;

import com.google.common.annotations.GwtIncompatible;
import java.io.Serializable;
import java.util.Collection;
import java.util.Comparator;
import java.util.Iterator;
import java.util.NavigableSet;
import java.util.SortedSet;
import java.util.TreeSet;


@GwtIncompatible
public final class SafeTreeSet<E> implements Serializable, NavigableSet<E> {
  @SuppressWarnings("unchecked")
  private static final Comparator<Object> NATURAL_ORDER =
      new Comparator<Object>() {
        @Override
        public int compare(Object o1, Object o2) {
          return ((Comparable<Object>) o1).compareTo(o2);
        }
      };

  private final NavigableSet<E> delegate;

  public SafeTreeSet() {
    this(new TreeSet<E>());
  }

  public SafeTreeSet(Collection<? extends E> collection) {
    this(new TreeSet<E>(collection));
  }

  public SafeTreeSet(Comparator<? super E> comparator) {
    this(new TreeSet<E>(comparator));
  }

  public SafeTreeSet(SortedSet<E> set) {
    this(new TreeSet<E>(set));
  }

  private SafeTreeSet(NavigableSet<E> delegate) {
    this.delegate = delegate;
    for (E e : this) {
      checkValid(e);
    }
  }

  @Override
  public boolean add(E element) {
    return delegate.add(checkValid(element));
  }

  @Override
  public boolean addAll(Collection<? extends E> collection) {
    for (E e : collection) {
      checkValid(e);
    }
    return delegate.addAll(collection);
  }

  @Override
  public E ceiling(E e) {
    return delegate.ceiling(checkValid(e));
  }

  @Override
  public void clear() {
    delegate.clear();
  }

  @SuppressWarnings("unchecked")
  @Override
  public Comparator<? super E> comparator() {
    Comparator<? super E> comparator = delegate.comparator();
    if (comparator == null) {
      comparator = (Comparator<? super E>) NATURAL_ORDER;
    }
    return comparator;
  }

  @Override
  public boolean contains(Object object) {
    return delegate.contains(checkValid(object));
  }

  @Override
  public boolean containsAll(Collection<?> c) {
    return delegate.containsAll(c);
  }

  @Override
  public Iterator<E> descendingIterator() {
    return delegate.descendingIterator();
  }

  @Override
  public NavigableSet<E> descendingSet() {
    return new SafeTreeSet<E>(delegate.descendingSet());
  }

  @Override
  public E first() {
    return delegate.first();
  }

  @Override
  public E floor(E e) {
    return delegate.floor(checkValid(e));
  }

  @Override
  public SortedSet<E> headSet(E toElement) {
    return headSet(toElement, false);
  }

  @Override
  public NavigableSet<E> headSet(E toElement, boolean inclusive) {
    return new SafeTreeSet<E>(delegate.headSet(checkValid(toElement), inclusive));
  }

  @Override
  public E higher(E e) {
    return delegate.higher(checkValid(e));
  }

  @Override
  public boolean isEmpty() {
    return delegate.isEmpty();
  }

  @Override
  public Iterator<E> iterator() {
    return delegate.iterator();
  }

  @Override
  public E last() {
    return delegate.last();
  }

  @Override
  public E lower(E e) {
    return delegate.lower(checkValid(e));
  }

  @Override
  public E pollFirst() {
    return delegate.pollFirst();
  }

  @Override
  public E pollLast() {
    return delegate.pollLast();
  }

  @Override
  public boolean remove(Object object) {
    return delegate.remove(checkValid(object));
  }

  @Override
  public boolean removeAll(Collection<?> c) {
    return delegate.removeAll(c);
  }

  @Override
  public boolean retainAll(Collection<?> c) {
    return delegate.retainAll(c);
  }

  @Override
  public int size() {
    return delegate.size();
  }

  @Override
  public NavigableSet<E> subSet(
      E fromElement, boolean fromInclusive, E toElement, boolean toInclusive) {
    return new SafeTreeSet<E>(
        delegate.subSet(
            checkValid(fromElement), fromInclusive, checkValid(toElement), toInclusive));
  }

  @Override
  public SortedSet<E> subSet(E fromElement, E toElement) {
    return subSet(fromElement, true, toElement, false);
  }

  @Override
  public SortedSet<E> tailSet(E fromElement) {
    return tailSet(fromElement, true);
  }

  @Override
  public NavigableSet<E> tailSet(E fromElement, boolean inclusive) {
    return new SafeTreeSet<E>(delegate.tailSet(checkValid(fromElement), inclusive));
  }

  @Override
  public Object[] toArray() {
    return delegate.toArray();
  }

  @Override
  public <T> T[] toArray(T[] a) {
    return delegate.toArray(a);
  }

  private <T> T checkValid(T t) {
        @SuppressWarnings("unchecked")
    E e = (E) t;
    comparator().compare(e, e);
    return t;
  }

  @Override
  public boolean equals(Object obj) {
    return delegate.equals(obj);
  }

  @Override
  public int hashCode() {
    return delegate.hashCode();
  }

  @Override
  public String toString() {
    return delegate.toString();
  }

  private static final long serialVersionUID = 0L;
}
