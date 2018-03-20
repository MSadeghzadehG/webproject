

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.GwtCompatible;
import java.util.Comparator;
import java.util.SortedSet;


@GwtCompatible
final class SortedIterables {
  private SortedIterables() {}

  
  public static boolean hasSameComparator(Comparator<?> comparator, Iterable<?> elements) {
    checkNotNull(comparator);
    checkNotNull(elements);
    Comparator<?> comparator2;
    if (elements instanceof SortedSet) {
      comparator2 = comparator((SortedSet<?>) elements);
    } else if (elements instanceof SortedIterable) {
      comparator2 = ((SortedIterable<?>) elements).comparator();
    } else {
      return false;
    }
    return comparator.equals(comparator2);
  }

  @SuppressWarnings("unchecked")
    public static <E> Comparator<? super E> comparator(SortedSet<E> sortedSet) {
    Comparator<? super E> result = sortedSet.comparator();
    if (result == null) {
      result = (Comparator<? super E>) Ordering.natural();
    }
    return result;
  }
}
