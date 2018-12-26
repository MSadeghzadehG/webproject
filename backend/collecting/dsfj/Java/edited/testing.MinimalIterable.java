

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;


@GwtCompatible
public final class MinimalIterable<E> implements Iterable<E> {
  
  public static <E> MinimalIterable<E> of(E... elements) {
        return new MinimalIterable<E>(Arrays.asList(elements).iterator());
  }

  
  @SuppressWarnings("unchecked")   public static <E> MinimalIterable<E> from(final Collection<E> elements) {
    return (MinimalIterable) of(elements.toArray());
  }

  private Iterator<E> iterator;

  private MinimalIterable(Iterator<E> iterator) {
    this.iterator = iterator;
  }

  @Override
  public Iterator<E> iterator() {
    if (iterator == null) {
                  throw new IllegalStateException();
    }
    try {
      return iterator;
    } finally {
      iterator = null;
    }
  }
}
