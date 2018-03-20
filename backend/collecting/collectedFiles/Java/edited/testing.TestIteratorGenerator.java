

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.Iterator;


@GwtCompatible
public interface TestIteratorGenerator<E> {
  Iterator<E> get();
}
