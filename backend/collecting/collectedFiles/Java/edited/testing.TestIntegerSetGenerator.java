

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.SampleElements.Ints;
import java.util.List;
import java.util.Set;


@GwtCompatible
public abstract class TestIntegerSetGenerator implements TestSetGenerator<Integer> {
  @Override
  public SampleElements<Integer> samples() {
    return new Ints();
  }

  @Override
  public Set<Integer> create(Object... elements) {
    Integer[] array = new Integer[elements.length];
    int i = 0;
    for (Object e : elements) {
      array[i++] = (Integer) e;
    }
    return create(array);
  }

  protected abstract Set<Integer> create(Integer[] elements);

  @Override
  public Integer[] createArray(int length) {
    return new Integer[length];
  }

  
  @Override
  public List<Integer> order(List<Integer> insertionOrder) {
    return insertionOrder;
  }
}
