

package com.google.common.base;

import com.google.common.annotations.GwtCompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
@FunctionalInterface
public interface Function<F, T> extends java.util.function.Function<F, T> {
  @Override
  @NullableDecl
  @CanIgnoreReturnValue   T apply(@NullableDecl F input);

  
  @Override
  boolean equals(@NullableDecl Object object);
}
