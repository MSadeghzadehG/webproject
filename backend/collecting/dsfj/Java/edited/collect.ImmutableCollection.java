

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;

import java.io.Serializable;
import java.util.AbstractCollection;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.Spliterator;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@SuppressWarnings("serial") public abstract class ImmutableCollection<E> extends AbstractCollection<E> implements Serializable {
  static final int SPLITERATOR_CHARACTERISTICS =
      Spliterator.IMMUTABLE | Spliterator.NONNULL | Spliterator.ORDERED;

  static final ImmutableCollection<Object> EMPTY_IMMUTABLE_COLLECTION =
      new ForwardingImmutableCollection<Object>(Collections.emptyList());

  ImmutableCollection() {}

  public abstract UnmodifiableIterator<E> iterator();

  public boolean contains(@NullableDecl Object object) {
    return object != null && super.contains(object);
  }

  public final boolean add(E e) {
    throw new UnsupportedOperationException();
  }

  public final boolean remove(Object object) {
    throw new UnsupportedOperationException();
  }

  public final boolean addAll(Collection<? extends E> newElements) {
    throw new UnsupportedOperationException();
  }

  public final boolean removeAll(Collection<?> oldElements) {
    throw new UnsupportedOperationException();
  }

  public final boolean retainAll(Collection<?> elementsToKeep) {
    throw new UnsupportedOperationException();
  }

  public final void clear() {
    throw new UnsupportedOperationException();
  }

  private transient ImmutableList<E> asList;

  public ImmutableList<E> asList() {
    ImmutableList<E> list = asList;
    return (list == null) ? (asList = createAsList()) : list;
  }

  ImmutableList<E> createAsList() {
    switch (size()) {
      case 0:
        return ImmutableList.of();
      case 1:
        return ImmutableList.of(iterator().next());
      default:
        return new RegularImmutableAsList<E>(this, toArray());
    }
  }

  static <E> ImmutableCollection<E> unsafeDelegate(Collection<E> delegate) {
    return new ForwardingImmutableCollection<E>(delegate);
  }

  boolean isPartialView() {
    return false;
  }

  
  public abstract static class Builder<E> {

    Builder() {}

    static int expandedCapacity(int oldCapacity, int minCapacity) {
      if (minCapacity < 0) { 
        throw new AssertionError("cannot store more than MAX_VALUE elements");
      }
            int newCapacity = oldCapacity + (oldCapacity >> 1) + 1;
      if (newCapacity < minCapacity) {
        newCapacity = Integer.highestOneBit(minCapacity - 1) << 1;
      }
      if (newCapacity < 0) {
        newCapacity = Integer.MAX_VALUE;
              }
      return newCapacity;
    }

    public abstract Builder<E> add(E element);

    public Builder<E> add(E... elements) {
      checkNotNull(elements);       for (E element : elements) {
        add(checkNotNull(element));
      }
      return this;
    }

    public Builder<E> addAll(Iterable<? extends E> elements) {
      checkNotNull(elements);       for (E element : elements) {
        add(checkNotNull(element));
      }
      return this;
    }

    public Builder<E> addAll(Iterator<? extends E> elements) {
      checkNotNull(elements);       while (elements.hasNext()) {
        add(checkNotNull(elements.next()));
      }
      return this;
    }

    public abstract ImmutableCollection<E> build();
  }
}
