

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.collect.CollectPreconditions.checkNonnegative;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import com.google.common.base.Function;
import com.google.common.base.Predicate;
import com.google.common.base.Predicates;
import com.google.common.math.IntMath;
import com.google.common.primitives.Ints;
import java.util.AbstractCollection;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.Spliterator;
import java.util.function.Consumer;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
public final class Collections2 {
  private Collections2() {}

  
      public static <E> Collection<E> filter(Collection<E> unfiltered, Predicate<? super E> predicate) {
    if (unfiltered instanceof FilteredCollection) {
                  return ((FilteredCollection<E>) unfiltered).createCombined(predicate);
    }

    return new FilteredCollection<E>(checkNotNull(unfiltered), checkNotNull(predicate));
  }

  
  static boolean safeContains(Collection<?> collection, @NullableDecl Object object) {
    checkNotNull(collection);
    try {
      return collection.contains(object);
    } catch (ClassCastException | NullPointerException e) {
      return false;
    }
  }

  
  static boolean safeRemove(Collection<?> collection, @NullableDecl Object object) {
    checkNotNull(collection);
    try {
      return collection.remove(object);
    } catch (ClassCastException | NullPointerException e) {
      return false;
    }
  }

  static class FilteredCollection<E> extends AbstractCollection<E> {
    final Collection<E> unfiltered;
    final Predicate<? super E> predicate;

    FilteredCollection(Collection<E> unfiltered, Predicate<? super E> predicate) {
      this.unfiltered = unfiltered;
      this.predicate = predicate;
    }

    FilteredCollection<E> createCombined(Predicate<? super E> newPredicate) {
      return new FilteredCollection<E>(unfiltered, Predicates.<E>and(predicate, newPredicate));
          }

    @Override
    public boolean add(E element) {
      checkArgument(predicate.apply(element));
      return unfiltered.add(element);
    }

    @Override
    public boolean addAll(Collection<? extends E> collection) {
      for (E element : collection) {
        checkArgument(predicate.apply(element));
      }
      return unfiltered.addAll(collection);
    }

    @Override
    public void clear() {
      Iterables.removeIf(unfiltered, predicate);
    }

    @Override
    public boolean contains(@NullableDecl Object element) {
      if (safeContains(unfiltered, element)) {
        @SuppressWarnings("unchecked")         E e = (E) element;
        return predicate.apply(e);
      }
      return false;
    }

    @Override
    public boolean containsAll(Collection<?> collection) {
      return containsAllImpl(this, collection);
    }

    @Override
    public boolean isEmpty() {
      return !Iterables.any(unfiltered, predicate);
    }

    @Override
    public Iterator<E> iterator() {
      return Iterators.filter(unfiltered.iterator(), predicate);
    }

    @Override
    public Spliterator<E> spliterator() {
      return CollectSpliterators.filter(unfiltered.spliterator(), predicate);
    }

    @Override
    public void forEach(Consumer<? super E> action) {
      checkNotNull(action);
      unfiltered.forEach(
          (E e) -> {
            if (predicate.test(e)) {
              action.accept(e);
            }
          });
    }

    @Override
    public boolean remove(Object element) {
      return contains(element) && unfiltered.remove(element);
    }

    @Override
    public boolean removeAll(final Collection<?> collection) {
      return removeIf(collection::contains);
    }

    @Override
    public boolean retainAll(final Collection<?> collection) {
      return removeIf(element -> !collection.contains(element));
    }

    @Override
    public boolean removeIf(java.util.function.Predicate<? super E> filter) {
      checkNotNull(filter);
      return unfiltered.removeIf(element -> predicate.apply(element) && filter.test(element));
    }

    @Override
    public int size() {
      int size = 0;
      for (E e : unfiltered) {
        if (predicate.apply(e)) {
          size++;
        }
      }
      return size;
    }

    @Override
    public Object[] toArray() {
            return Lists.newArrayList(iterator()).toArray();
    }

    @Override
    public <T> T[] toArray(T[] array) {
      return Lists.newArrayList(iterator()).toArray(array);
    }
  }

  
  public static <F, T> Collection<T> transform(
      Collection<F> fromCollection, Function<? super F, T> function) {
    return new TransformedCollection<>(fromCollection, function);
  }

  static class TransformedCollection<F, T> extends AbstractCollection<T> {
    final Collection<F> fromCollection;
    final Function<? super F, ? extends T> function;

    TransformedCollection(Collection<F> fromCollection, Function<? super F, ? extends T> function) {
      this.fromCollection = checkNotNull(fromCollection);
      this.function = checkNotNull(function);
    }

    @Override
    public void clear() {
      fromCollection.clear();
    }

    @Override
    public boolean isEmpty() {
      return fromCollection.isEmpty();
    }

    @Override
    public Iterator<T> iterator() {
      return Iterators.transform(fromCollection.iterator(), function);
    }

    @Override
    public Spliterator<T> spliterator() {
      return CollectSpliterators.map(fromCollection.spliterator(), function);
    }

    @Override
    public void forEach(Consumer<? super T> action) {
      checkNotNull(action);
      fromCollection.forEach((F f) -> action.accept(function.apply(f)));
    }

    @Override
    public boolean removeIf(java.util.function.Predicate<? super T> filter) {
      checkNotNull(filter);
      return fromCollection.removeIf(element -> filter.test(function.apply(element)));
    }

    @Override
    public int size() {
      return fromCollection.size();
    }
  }

  
  static boolean containsAllImpl(Collection<?> self, Collection<?> c) {
    for (Object o : c) {
      if (!self.contains(o)) {
        return false;
      }
    }
    return true;
  }

  
  static String toStringImpl(final Collection<?> collection) {
    StringBuilder sb = newStringBuilderForCollection(collection.size()).append('[');
    boolean first = true;
    for (Object o : collection) {
      if (!first) {
        sb.append(", ");
      }
      first = false;
      if (o == collection) {
        sb.append("(this Collection)");
      } else {
        sb.append(o);
      }
    }
    return sb.append(']').toString();
  }

  
  static StringBuilder newStringBuilderForCollection(int size) {
    checkNonnegative(size, "size");
    return new StringBuilder((int) Math.min(size * 8L, Ints.MAX_POWER_OF_TWO));
  }

  
  @Beta
  public static <E extends Comparable<? super E>> Collection<List<E>> orderedPermutations(
      Iterable<E> elements) {
    return orderedPermutations(elements, Ordering.natural());
  }

  
  @Beta
  public static <E> Collection<List<E>> orderedPermutations(
      Iterable<E> elements, Comparator<? super E> comparator) {
    return new OrderedPermutationCollection<E>(elements, comparator);
  }

  private static final class OrderedPermutationCollection<E> extends AbstractCollection<List<E>> {
    final ImmutableList<E> inputList;
    final Comparator<? super E> comparator;
    final int size;

    OrderedPermutationCollection(Iterable<E> input, Comparator<? super E> comparator) {
      this.inputList = ImmutableList.sortedCopyOf(comparator, input);
      this.comparator = comparator;
      this.size = calculateSize(inputList, comparator);
    }

    
    private static <E> int calculateSize(
        List<E> sortedInputList, Comparator<? super E> comparator) {
      int permutations = 1;
      int n = 1;
      int r = 1;
      while (n < sortedInputList.size()) {
        int comparison = comparator.compare(sortedInputList.get(n - 1), sortedInputList.get(n));
        if (comparison < 0) {
                    permutations = IntMath.saturatedMultiply(permutations, IntMath.binomial(n, r));
          r = 0;
          if (permutations == Integer.MAX_VALUE) {
            return Integer.MAX_VALUE;
          }
        }
        n++;
        r++;
      }
      return IntMath.saturatedMultiply(permutations, IntMath.binomial(n, r));
    }

    @Override
    public int size() {
      return size;
    }

    @Override
    public boolean isEmpty() {
      return false;
    }

    @Override
    public Iterator<List<E>> iterator() {
      return new OrderedPermutationIterator<E>(inputList, comparator);
    }

    @Override
    public boolean contains(@NullableDecl Object obj) {
      if (obj instanceof List) {
        List<?> list = (List<?>) obj;
        return isPermutation(inputList, list);
      }
      return false;
    }

    @Override
    public String toString() {
      return "orderedPermutationCollection(" + inputList + ")";
    }
  }

  private static final class OrderedPermutationIterator<E> extends AbstractIterator<List<E>> {
    @NullableDecl
    List<E> nextPermutation;
    final Comparator<? super E> comparator;

    OrderedPermutationIterator(List<E> list, Comparator<? super E> comparator) {
      this.nextPermutation = Lists.newArrayList(list);
      this.comparator = comparator;
    }

    @Override
    protected List<E> computeNext() {
      if (nextPermutation == null) {
        return endOfData();
      }
      ImmutableList<E> next = ImmutableList.copyOf(nextPermutation);
      calculateNextPermutation();
      return next;
    }

    void calculateNextPermutation() {
      int j = findNextJ();
      if (j == -1) {
        nextPermutation = null;
        return;
      }

      int l = findNextL(j);
      Collections.swap(nextPermutation, j, l);
      int n = nextPermutation.size();
      Collections.reverse(nextPermutation.subList(j + 1, n));
    }

    int findNextJ() {
      for (int k = nextPermutation.size() - 2; k >= 0; k--) {
        if (comparator.compare(nextPermutation.get(k), nextPermutation.get(k + 1)) < 0) {
          return k;
        }
      }
      return -1;
    }

    int findNextL(int j) {
      E ak = nextPermutation.get(j);
      for (int l = nextPermutation.size() - 1; l > j; l--) {
        if (comparator.compare(ak, nextPermutation.get(l)) < 0) {
          return l;
        }
      }
      throw new AssertionError("this statement should be unreachable");
    }
  }

  
  @Beta
  public static <E> Collection<List<E>> permutations(Collection<E> elements) {
    return new PermutationCollection<E>(ImmutableList.copyOf(elements));
  }

  private static final class PermutationCollection<E> extends AbstractCollection<List<E>> {
    final ImmutableList<E> inputList;

    PermutationCollection(ImmutableList<E> input) {
      this.inputList = input;
    }

    @Override
    public int size() {
      return IntMath.factorial(inputList.size());
    }

    @Override
    public boolean isEmpty() {
      return false;
    }

    @Override
    public Iterator<List<E>> iterator() {
      return new PermutationIterator<E>(inputList);
    }

    @Override
    public boolean contains(@NullableDecl Object obj) {
      if (obj instanceof List) {
        List<?> list = (List<?>) obj;
        return isPermutation(inputList, list);
      }
      return false;
    }

    @Override
    public String toString() {
      return "permutations(" + inputList + ")";
    }
  }

  private static class PermutationIterator<E> extends AbstractIterator<List<E>> {
    final List<E> list;
    final int[] c;
    final int[] o;
    int j;

    PermutationIterator(List<E> list) {
      this.list = new ArrayList<E>(list);
      int n = list.size();
      c = new int[n];
      o = new int[n];
      Arrays.fill(c, 0);
      Arrays.fill(o, 1);
      j = Integer.MAX_VALUE;
    }

    @Override
    protected List<E> computeNext() {
      if (j <= 0) {
        return endOfData();
      }
      ImmutableList<E> next = ImmutableList.copyOf(list);
      calculateNextPermutation();
      return next;
    }

    void calculateNextPermutation() {
      j = list.size() - 1;
      int s = 0;

                  if (j == -1) {
        return;
      }

      while (true) {
        int q = c[j] + o[j];
        if (q < 0) {
          switchDirection();
          continue;
        }
        if (q == j + 1) {
          if (j == 0) {
            break;
          }
          s++;
          switchDirection();
          continue;
        }

        Collections.swap(list, j - c[j] + s, j - q + s);
        c[j] = q;
        break;
      }
    }

    void switchDirection() {
      o[j] = -o[j];
      j--;
    }
  }

  
  private static boolean isPermutation(List<?> first, List<?> second) {
    if (first.size() != second.size()) {
      return false;
    }
    Multiset<?> firstMultiset = HashMultiset.create(first);
    Multiset<?> secondMultiset = HashMultiset.create(second);
    return firstMultiset.equals(secondMultiset);
  }
}
