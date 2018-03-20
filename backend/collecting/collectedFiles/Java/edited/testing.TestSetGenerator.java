

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.Set;


@GwtCompatible
public interface TestSetGenerator<E> extends TestCollectionGenerator<E> {
  @Override
  Set<E> create(Object... elements);
}
