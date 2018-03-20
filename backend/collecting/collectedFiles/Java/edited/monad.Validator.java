
package com.iluwatar.monad;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.function.Function;
import java.util.function.Predicate;


public class Validator<T> {
  
  private final T t;

  
  private final List<Throwable> exceptions = new ArrayList<>();

  
  private Validator(T t) {
    this.t = t;
  }

  
  public static <T> Validator<T> of(T t) {
    return new Validator<>(Objects.requireNonNull(t));
  }

  
  public Validator<T> validate(Predicate<T> validation, String message) {
    if (!validation.test(t)) {
      exceptions.add(new IllegalStateException(message));
    }
    return this;
  }

  
  public <U> Validator<T> validate(Function<T, U> projection, Predicate<U> validation,
                                   String message) {
    return validate(projection.andThen(validation::test)::apply, message);
  }

  
  public T get() throws IllegalStateException {
    if (exceptions.isEmpty()) {
      return t;
    }
    IllegalStateException e = new IllegalStateException();
    exceptions.forEach(e::addSuppressed);
    throw e;
  }
}
