

package com.google.common.collect;

import com.google.common.annotations.Beta;
import java.util.Comparator;
import java.util.SortedSet;


@Beta
public interface SortedMultiset<E> extends Multiset<E>, SortedIterable<E> {
  Comparator<? super E> comparator();

  Entry<E> firstEntry();

  Entry<E> lastEntry();

  Entry<E> pollFirstEntry();

  Entry<E> pollLastEntry();

  
  @Override
  SortedSet<E> elementSet();

  SortedMultiset<E> descendingMultiset();

  SortedMultiset<E> headMultiset(E upperBound, BoundType boundType);

  SortedMultiset<E> subMultiset(
      E lowerBound, BoundType lowerBoundType, E upperBound, BoundType upperBoundType);

  SortedMultiset<E> tailMultiset(E lowerBound, BoundType boundType);
}
