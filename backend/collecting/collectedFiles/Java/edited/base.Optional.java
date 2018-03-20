

package com.google.common.base;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import java.io.Serializable;
import java.util.Iterator;
import java.util.Set;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible(serializable = true)
public abstract class Optional<T> implements Serializable {
  
  public static <T> Optional<T> absent() {
    return Absent.withType();
  }

  
  public static <T> Optional<T> of(T reference) {
    return new Present<T>(checkNotNull(reference));
  }

  
  public static <T> Optional<T> fromNullable(@NullableDecl T nullableReference) {
    return (nullableReference == null) ? Optional.<T>absent() : new Present<T>(nullableReference);
  }

  
  @NullableDecl
  public static <T> Optional<T> fromJavaUtil(@NullableDecl java.util.Optional<T> javaUtilOptional) {
    return (javaUtilOptional == null) ? null : fromNullable(javaUtilOptional.orElse(null));
  }

  
  @NullableDecl
  public static <T> java.util.Optional<T> toJavaUtil(@NullableDecl Optional<T> googleOptional) {
    return googleOptional == null ? null : googleOptional.toJavaUtil();
  }

  
  public java.util.Optional<T> toJavaUtil() {
    return java.util.Optional.ofNullable(orNull());
  }

  Optional() {}

  
  public abstract boolean isPresent();

  
  public abstract T get();

  
  public abstract T or(T defaultValue);

  
  public abstract Optional<T> or(Optional<? extends T> secondChoice);

  
  @Beta
  public abstract T or(Supplier<? extends T> supplier);

  
  @NullableDecl
  public abstract T orNull();

  
  public abstract Set<T> asSet();

  
  public abstract <V> Optional<V> transform(Function<? super T, V> function);

  
  @Override
  public abstract boolean equals(@NullableDecl Object object);

  
  @Override
  public abstract int hashCode();

  
  @Override
  public abstract String toString();

  
  @Beta
  public static <T> Iterable<T> presentInstances(
      final Iterable<? extends Optional<? extends T>> optionals) {
    checkNotNull(optionals);
    return new Iterable<T>() {
      @Override
      public Iterator<T> iterator() {
        return new AbstractIterator<T>() {
          private final Iterator<? extends Optional<? extends T>> iterator =
              checkNotNull(optionals.iterator());

          @Override
          protected T computeNext() {
            while (iterator.hasNext()) {
              Optional<? extends T> optional = iterator.next();
              if (optional.isPresent()) {
                return optional.get();
              }
            }
            return endOfData();
          }
        };
      }
    };
  }

  private static final long serialVersionUID = 0;
}
