

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.collect.CollectPreconditions.checkNonnegative;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collector;


@Beta
@GwtCompatible
public final class Comparators {
  private Comparators() {}

  
        public static <T, S extends T> Comparator<Iterable<S>> lexicographical(Comparator<T> comparator) {
    return new LexicographicalOrdering<S>(checkNotNull(comparator));
  }

  
  public static <T> boolean isInOrder(Iterable<? extends T> iterable, Comparator<T> comparator) {
    checkNotNull(comparator);
    Iterator<? extends T> it = iterable.iterator();
    if (it.hasNext()) {
      T prev = it.next();
      while (it.hasNext()) {
        T next = it.next();
        if (comparator.compare(prev, next) > 0) {
          return false;
        }
        prev = next;
      }
    }
    return true;
  }

  
  public static <T> boolean isInStrictOrder(
      Iterable<? extends T> iterable, Comparator<T> comparator) {
    checkNotNull(comparator);
    Iterator<? extends T> it = iterable.iterator();
    if (it.hasNext()) {
      T prev = it.next();
      while (it.hasNext()) {
        T next = it.next();
        if (comparator.compare(prev, next) >= 0) {
          return false;
        }
        prev = next;
      }
    }
    return true;
  }

  
  public static <T> Collector<T, ?, List<T>> least(int k, Comparator<? super T> comparator) {
    checkNonnegative(k, "k");
    checkNotNull(comparator);
    return Collector.of(
        () -> TopKSelector.<T>least(k, comparator),
        TopKSelector::offer,
        TopKSelector::combine,
        TopKSelector::topK,
        Collector.Characteristics.UNORDERED);
  }

  
  public static <T> Collector<T, ?, List<T>> greatest(int k, Comparator<? super T> comparator) {
    return least(k, comparator.reversed());
  }

  
  @Beta
  public static <T> Comparator<Optional<T>> emptiesFirst(Comparator<? super T> valueComparator) {
    checkNotNull(valueComparator);
    return Comparator.comparing(o -> o.orElse(null), Comparator.nullsFirst(valueComparator));
  }

  
  @Beta
  public static <T> Comparator<Optional<T>> emptiesLast(Comparator<? super T> valueComparator) {
    checkNotNull(valueComparator);
    return Comparator.comparing(o -> o.orElse(null), Comparator.nullsLast(valueComparator));
  }
}
