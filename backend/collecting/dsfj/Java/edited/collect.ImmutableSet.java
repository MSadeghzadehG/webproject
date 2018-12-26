

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.Beta;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.stream.Collector;


@SuppressWarnings("serial") public abstract class ImmutableSet<E> extends ImmutableCollection<E> implements Set<E> {
  ImmutableSet() {}

  @Beta
  public static <E> Collector<E, ?, ImmutableSet<E>> toImmutableSet() {
    return CollectCollectors.toImmutableSet();
  }

    @SuppressWarnings({"unchecked"})
  public static <E> ImmutableSet<E> of() {
    return (ImmutableSet<E>) RegularImmutableSet.EMPTY;
  }

  public static <E> ImmutableSet<E> of(E element) {
    return new SingletonImmutableSet<E>(element);
  }

  @SuppressWarnings("unchecked")
  public static <E> ImmutableSet<E> of(E e1, E e2) {
    return create(e1, e2);
  }

  @SuppressWarnings("unchecked")
  public static <E> ImmutableSet<E> of(E e1, E e2, E e3) {
    return create(e1, e2, e3);
  }

  @SuppressWarnings("unchecked")
  public static <E> ImmutableSet<E> of(E e1, E e2, E e3, E e4) {
    return create(e1, e2, e3, e4);
  }

  @SuppressWarnings("unchecked")
  public static <E> ImmutableSet<E> of(E e1, E e2, E e3, E e4, E e5) {
    return create(e1, e2, e3, e4, e5);
  }

  @SuppressWarnings("unchecked")
  public static <E> ImmutableSet<E> of(E e1, E e2, E e3, E e4, E e5, E e6, E... others) {
    int size = others.length + 6;
    List<E> all = new ArrayList<E>(size);
    Collections.addAll(all, e1, e2, e3, e4, e5, e6);
    Collections.addAll(all, others);
    return copyOf(all.iterator());
  }

  public static <E> ImmutableSet<E> copyOf(E[] elements) {
    checkNotNull(elements);
    switch (elements.length) {
      case 0:
        return of();
      case 1:
        return of(elements[0]);
      default:
        return create(elements);
    }
  }

  public static <E> ImmutableSet<E> copyOf(Collection<? extends E> elements) {
    Iterable<? extends E> iterable = elements;
    return copyOf(iterable);
  }

  public static <E> ImmutableSet<E> copyOf(Iterable<? extends E> elements) {
    if (elements instanceof ImmutableSet && !(elements instanceof ImmutableSortedSet)) {
      @SuppressWarnings("unchecked")       ImmutableSet<E> set = (ImmutableSet<E>) elements;
      return set;
    }
    return copyOf(elements.iterator());
  }

  public static <E> ImmutableSet<E> copyOf(Iterator<? extends E> elements) {
    if (!elements.hasNext()) {
      return of();
    }
    E first = elements.next();
    if (!elements.hasNext()) {
            return ImmutableSet.<E>of(first);
    }

    Set<E> delegate = Sets.newLinkedHashSet();
    delegate.add(checkNotNull(first));
    do {
      delegate.add(checkNotNull(elements.next()));
    } while (elements.hasNext());

    return unsafeDelegate(delegate);
  }

      static <E> ImmutableSet<E> unsafeDelegate(Set<E> delegate) {
    switch (delegate.size()) {
      case 0:
        return of();
      case 1:
        return new SingletonImmutableSet<E>(delegate.iterator().next());
      default:
        return new RegularImmutableSet<E>(delegate);
    }
  }

  private static <E> ImmutableSet<E> create(E... elements) {
        Set<E> set = Sets.newLinkedHashSet();
    Collections.addAll(set, elements);
    for (E element : set) {
      checkNotNull(element);
    }

    switch (set.size()) {
      case 0:
        return of();
      case 1:
        return new SingletonImmutableSet<E>(set.iterator().next());
      default:
        return new RegularImmutableSet<E>(set);
    }
  }

  @Override
  public boolean equals(Object obj) {
    return Sets.equalsImpl(this, obj);
  }

  @Override
  public int hashCode() {
    return Sets.hashCodeImpl(this);
  }

      @Override
  public abstract UnmodifiableIterator<E> iterator();

  abstract static class Indexed<E> extends ImmutableSet<E> {
    abstract E get(int index);

    @Override
    public UnmodifiableIterator<E> iterator() {
      return asList().iterator();
    }

    @Override
    ImmutableList<E> createAsList() {
      return new ImmutableAsList<E>() {
        @Override
        public E get(int index) {
          return Indexed.this.get(index);
        }

        @Override
        Indexed<E> delegateCollection() {
          return Indexed.this;
        }
      };
    }
  }

  public static <E> Builder<E> builder() {
    return new Builder<E>();
  }

  public static <E> Builder<E> builderWithExpectedSize(int size) {
    return new Builder<E>(size);
  }

  public static class Builder<E> extends ImmutableCollection.Builder<E> {
        final ArrayList<E> contents;

    public Builder() {
      this.contents = Lists.newArrayList();
    }

    Builder(int initialCapacity) {
      this.contents = Lists.newArrayListWithCapacity(initialCapacity);
    }

    @Override
    public Builder<E> add(E element) {
      contents.add(checkNotNull(element));
      return this;
    }

    @Override
    public Builder<E> add(E... elements) {
      checkNotNull(elements);       contents.ensureCapacity(contents.size() + elements.length);
      super.add(elements);
      return this;
    }

    @Override
    public Builder<E> addAll(Iterable<? extends E> elements) {
      if (elements instanceof Collection) {
        Collection<?> collection = (Collection<?>) elements;
        contents.ensureCapacity(contents.size() + collection.size());
      }
      super.addAll(elements);
      return this;
    }

    @Override
    public Builder<E> addAll(Iterator<? extends E> elements) {
      super.addAll(elements);
      return this;
    }

    Builder<E> combine(Builder<E> builder) {
      contents.addAll(builder.contents);
      return this;
    }

    @Override
    public ImmutableSet<E> build() {
      return copyOf(contents.iterator());
    }
  }
}
