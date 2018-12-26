

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.Collection;
import java.util.List;
import java.util.Map;


@GwtCompatible
public interface TestContainerGenerator<T, E> {
  
  SampleElements<E> samples();

  
  T create(Object... elements);

  
  E[] createArray(int length);

  
  Iterable<E> order(List<E> insertionOrder);
}
