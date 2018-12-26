

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.Collections;
import java.util.List;
import java.util.SortedSet;


@GwtCompatible
public abstract class TestIntegerSortedSetGenerator extends TestIntegerSetGenerator {
  @Override
  protected abstract SortedSet<Integer> create(Integer[] elements);

  
  @Override
  public List<Integer> order(List<Integer> insertionOrder) {
    Collections.sort(insertionOrder);
    return insertionOrder;
  }
}
