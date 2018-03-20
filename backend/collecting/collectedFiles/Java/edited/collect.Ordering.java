

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.collect.CollectPreconditions.checkNonnegative;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.VisibleForTesting;
import com.google.common.base.Function;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;
import java.util.NoSuchElementException;
import java.util.SortedMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.atomic.AtomicInteger;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
public abstract class Ordering<T> implements Comparator<T> {
  
  
  @GwtCompatible(serializable = true)
  @SuppressWarnings("unchecked")   public static <C extends Comparable> Ordering<C> natural() {
    return (Ordering<C>) NaturalOrdering.INSTANCE;
  }

  
  
  @GwtCompatible(serializable = true)
  public static <T> Ordering<T> from(Comparator<T> comparator) {
    return (comparator instanceof Ordering)
        ? (Ordering<T>) comparator
        : new ComparatorOrdering<T>(comparator);
  }

  
  @GwtCompatible(serializable = true)
  @Deprecated
  public static <T> Ordering<T> from(Ordering<T> ordering) {
    return checkNotNull(ordering);
  }

  
    @GwtCompatible(serializable = true)
  public static <T> Ordering<T> explicit(List<T> valuesInOrder) {
    return new ExplicitOrdering<T>(valuesInOrder);
  }

  
    @GwtCompatible(serializable = true)
  public static <T> Ordering<T> explicit(T leastValue, T... remainingValuesInOrder) {
    return explicit(Lists.asList(leastValue, remainingValuesInOrder));
  }

  
  
  @GwtCompatible(serializable = true)
  @SuppressWarnings("unchecked")
  public static Ordering<Object> allEqual() {
    return AllEqualOrdering.INSTANCE;
  }

  
  @GwtCompatible(serializable = true)
  public static Ordering<Object> usingToString() {
    return UsingToStringOrdering.INSTANCE;
  }

  
    public static Ordering<Object> arbitrary() {
    return ArbitraryOrderingHolder.ARBITRARY_ORDERING;
  }

  private static class ArbitraryOrderingHolder {
    static final Ordering<Object> ARBITRARY_ORDERING = new ArbitraryOrdering();
  }

  @VisibleForTesting
  static class ArbitraryOrdering extends Ordering<Object> {

    private final AtomicInteger counter = new AtomicInteger(0);
    private final ConcurrentMap<Object, Integer> uids =
        Platform.tryWeakKeys(new MapMaker()).makeMap();

    private Integer getUid(Object obj) {
      Integer uid = uids.get(obj);
      if (uid == null) {
                                uid = counter.getAndIncrement();
        Integer alreadySet = uids.putIfAbsent(obj, uid);
        if (alreadySet != null) {
          uid = alreadySet;
        }
      }
      return uid;
    }

    @Override
    public int compare(Object left, Object right) {
      if (left == right) {
        return 0;
      } else if (left == null) {
        return -1;
      } else if (right == null) {
        return 1;
      }
      int leftCode = identityHashCode(left);
      int rightCode = identityHashCode(right);
      if (leftCode != rightCode) {
        return leftCode < rightCode ? -1 : 1;
      }

            int result = getUid(left).compareTo(getUid(right));
      if (result == 0) {
        throw new AssertionError();       }
      return result;
    }

    @Override
    public String toString() {
      return "Ordering.arbitrary()";
    }

    
    int identityHashCode(Object object) {
      return System.identityHashCode(object);
    }
  }

  
  
  protected Ordering() {}

  
  
      @GwtCompatible(serializable = true)
  public <S extends T> Ordering<S> reverse() {
    return new ReverseOrdering<S>(this);
  }

  
      @GwtCompatible(serializable = true)
  public <S extends T> Ordering<S> nullsFirst() {
    return new NullsFirstOrdering<S>(this);
  }

  
      @GwtCompatible(serializable = true)
  public <S extends T> Ordering<S> nullsLast() {
    return new NullsLastOrdering<S>(this);
  }

  
  @GwtCompatible(serializable = true)
  public <F> Ordering<F> onResultOf(Function<F, ? extends T> function) {
    return new ByFunctionOrdering<>(function, this);
  }

  <T2 extends T> Ordering<Entry<T2, ?>> onKeys() {
    return onResultOf(Maps.<T2>keyFunction());
  }

  
  @GwtCompatible(serializable = true)
  public <U extends T> Ordering<U> compound(Comparator<? super U> secondaryComparator) {
    return new CompoundOrdering<U>(this, checkNotNull(secondaryComparator));
  }

  
  @GwtCompatible(serializable = true)
  public static <T> Ordering<T> compound(Iterable<? extends Comparator<? super T>> comparators) {
    return new CompoundOrdering<T>(comparators);
  }

  
  @GwtCompatible(serializable = true)
        public <S extends T> Ordering<Iterable<S>> lexicographical() {
    
    return new LexicographicalOrdering<S>(this);
  }

  
    @CanIgnoreReturnValue   @Override
  public abstract int compare(@NullableDecl T left, @NullableDecl T right);

  
  @CanIgnoreReturnValue   public <E extends T> E min(Iterator<E> iterator) {
        E minSoFar = iterator.next();

    while (iterator.hasNext()) {
      minSoFar = min(minSoFar, iterator.next());
    }

    return minSoFar;
  }

  
  @CanIgnoreReturnValue   public <E extends T> E min(Iterable<E> iterable) {
    return min(iterable.iterator());
  }

  
  @CanIgnoreReturnValue   public <E extends T> E min(@NullableDecl E a, @NullableDecl E b) {
    return (compare(a, b) <= 0) ? a : b;
  }

  
  @CanIgnoreReturnValue   public <E extends T> E min(@NullableDecl E a, @NullableDecl E b, @NullableDecl E c, E... rest) {
    E minSoFar = min(min(a, b), c);

    for (E r : rest) {
      minSoFar = min(minSoFar, r);
    }

    return minSoFar;
  }

  
  @CanIgnoreReturnValue   public <E extends T> E max(Iterator<E> iterator) {
        E maxSoFar = iterator.next();

    while (iterator.hasNext()) {
      maxSoFar = max(maxSoFar, iterator.next());
    }

    return maxSoFar;
  }

  
  @CanIgnoreReturnValue   public <E extends T> E max(Iterable<E> iterable) {
    return max(iterable.iterator());
  }

  
  @CanIgnoreReturnValue   public <E extends T> E max(@NullableDecl E a, @NullableDecl E b) {
    return (compare(a, b) >= 0) ? a : b;
  }

  
  @CanIgnoreReturnValue   public <E extends T> E max(@NullableDecl E a, @NullableDecl E b, @NullableDecl E c, E... rest) {
    E maxSoFar = max(max(a, b), c);

    for (E r : rest) {
      maxSoFar = max(maxSoFar, r);
    }

    return maxSoFar;
  }

  
  public <E extends T> List<E> leastOf(Iterable<E> iterable, int k) {
    if (iterable instanceof Collection) {
      Collection<E> collection = (Collection<E>) iterable;
      if (collection.size() <= 2L * k) {
                        
        @SuppressWarnings("unchecked")         E[] array = (E[]) collection.toArray();
        Arrays.sort(array, this);
        if (array.length > k) {
          array = Arrays.copyOf(array, k);
        }
        return Collections.unmodifiableList(Arrays.asList(array));
      }
    }
    return leastOf(iterable.iterator(), k);
  }

  
  public <E extends T> List<E> leastOf(Iterator<E> iterator, int k) {
    checkNotNull(iterator);
    checkNonnegative(k, "k");

    if (k == 0 || !iterator.hasNext()) {
      return Collections.emptyList();
    } else if (k >= Integer.MAX_VALUE / 2) {
            ArrayList<E> list = Lists.newArrayList(iterator);
      Collections.sort(list, this);
      if (list.size() > k) {
        list.subList(k, list.size()).clear();
      }
      list.trimToSize();
      return Collections.unmodifiableList(list);
    } else {
      TopKSelector<E> selector = TopKSelector.least(k, this);
      selector.offerAll(iterator);
      return selector.topK();
    }
  }

  
  public <E extends T> List<E> greatestOf(Iterable<E> iterable, int k) {
            return reverse().leastOf(iterable, k);
  }

  
  public <E extends T> List<E> greatestOf(Iterator<E> iterator, int k) {
    return reverse().leastOf(iterator, k);
  }

  
    @CanIgnoreReturnValue   public <E extends T> List<E> sortedCopy(Iterable<E> elements) {
    @SuppressWarnings("unchecked")     E[] array = (E[]) Iterables.toArray(elements);
    Arrays.sort(array, this);
    return Lists.newArrayList(Arrays.asList(array));
  }

  
    @CanIgnoreReturnValue   public <E extends T> ImmutableList<E> immutableSortedCopy(Iterable<E> elements) {
    return ImmutableList.sortedCopyOf(this, elements);
  }

  
  public boolean isOrdered(Iterable<? extends T> iterable) {
    Iterator<? extends T> it = iterable.iterator();
    if (it.hasNext()) {
      T prev = it.next();
      while (it.hasNext()) {
        T next = it.next();
        if (compare(prev, next) > 0) {
          return false;
        }
        prev = next;
      }
    }
    return true;
  }

  
  public boolean isStrictlyOrdered(Iterable<? extends T> iterable) {
    Iterator<? extends T> it = iterable.iterator();
    if (it.hasNext()) {
      T prev = it.next();
      while (it.hasNext()) {
        T next = it.next();
        if (compare(prev, next) >= 0) {
          return false;
        }
        prev = next;
      }
    }
    return true;
  }

  
  @Deprecated
  public int binarySearch(List<? extends T> sortedList, @NullableDecl T key) {
    return Collections.binarySearch(sortedList, key, this);
  }

  
  @VisibleForTesting
  static class IncomparableValueException extends ClassCastException {
    final Object value;

    IncomparableValueException(Object value) {
      super("Cannot compare value: " + value);
      this.value = value;
    }

    private static final long serialVersionUID = 0;
  }

    static final int LEFT_IS_GREATER = 1;
  static final int RIGHT_IS_GREATER = -1;
}
