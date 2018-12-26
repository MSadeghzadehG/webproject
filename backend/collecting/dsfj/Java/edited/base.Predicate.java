

package com.google.common.base;

import com.google.common.annotations.GwtCompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
public interface Predicate<T> {
  
  @CanIgnoreReturnValue
  boolean apply(@NullableDecl T input);

  
  @Override
  boolean equals(@NullableDecl Object object);
}
