

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import java.util.Spliterator;
import java.util.function.Consumer;

@GwtCompatible(emulated = true)
abstract class IndexedImmutableSet<E> extends ImmutableSet<E> {
  abstract E get(int index);

  @Override
  public UnmodifiableIterator<E> iterator() {
    return asList().iterator();
  }

  @Override
  public Spliterator<E> spliterator() {
    return CollectSpliterators.indexed(size(), SPLITERATOR_CHARACTERISTICS, this::get);
  }

  @Override
  public void forEach(Consumer<? super E> consumer) {
    checkNotNull(consumer);
    int n = size();
    for (int i = 0; i < n; i++) {
      consumer.accept(get(i));
    }
  }

  @Override
  @GwtIncompatible
  int copyIntoArray(Object[] dst, int offset) {
    return asList().copyIntoArray(dst, offset);
  }

  @Override
  ImmutableList<E> createAsList() {
    return new ImmutableAsList<E>() {
      @Override
      public E get(int index) {
        return IndexedImmutableSet.this.get(index);
      }

      @Override
      boolean isPartialView() {
        return IndexedImmutableSet.this.isPartialView();
      }

      @Override
      public int size() {
        return IndexedImmutableSet.this.size();
      }

      @Override
      ImmutableCollection<E> delegateCollection() {
        return IndexedImmutableSet.this;
      }
    };
  }
}
