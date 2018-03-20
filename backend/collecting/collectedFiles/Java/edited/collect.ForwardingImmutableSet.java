

package com.google.common.collect;

import java.util.Collection;
import java.util.Collections;
import java.util.Set;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@SuppressWarnings("serial") public abstract class ForwardingImmutableSet<E> extends ImmutableSet<E> {
  private final transient Set<E> delegate;

  ForwardingImmutableSet(Set<E> delegate) {
        this.delegate = Collections.unmodifiableSet(delegate);
  }

  @Override
  public UnmodifiableIterator<E> iterator() {
    return Iterators.unmodifiableIterator(delegate.iterator());
  }

  @Override
  public boolean contains(@NullableDecl Object object) {
    return object != null && delegate.contains(object);
  }

  @Override
  public boolean containsAll(Collection<?> targets) {
    return delegate.containsAll(targets);
  }

  @Override
  public int size() {
    return delegate.size();
  }

  @Override
  public boolean isEmpty() {
    return delegate.isEmpty();
  }

  @Override
  public Object[] toArray() {
    return delegate.toArray();
  }

  @Override
  public <T> T[] toArray(T[] other) {
    return delegate.toArray(other);
  }

  @Override
  public String toString() {
    return delegate.toString();
  }

  
  @Override
  public int hashCode() {
    return delegate.hashCode();
  }
}
