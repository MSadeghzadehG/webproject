

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.VisibleForTesting;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.io.Serializable;
import java.util.ArrayDeque;
import java.util.Collection;
import java.util.Queue;


@Beta
@GwtCompatible
public final class EvictingQueue<E> extends ForwardingQueue<E> implements Serializable {

  private final Queue<E> delegate;

  @VisibleForTesting final int maxSize;

  private EvictingQueue(int maxSize) {
    checkArgument(maxSize >= 0, "maxSize (%s) must >= 0", maxSize);
    this.delegate = new ArrayDeque<E>(maxSize);
    this.maxSize = maxSize;
  }

  
  public static <E> EvictingQueue<E> create(int maxSize) {
    return new EvictingQueue<E>(maxSize);
  }

  
  public int remainingCapacity() {
    return maxSize - size();
  }

  @Override
  protected Queue<E> delegate() {
    return delegate;
  }

  
  @Override
  @CanIgnoreReturnValue
  public boolean offer(E e) {
    return add(e);
  }

  
  @Override
  @CanIgnoreReturnValue
  public boolean add(E e) {
    checkNotNull(e);     if (maxSize == 0) {
      return true;
    }
    if (size() == maxSize) {
      delegate.remove();
    }
    delegate.add(e);
    return true;
  }

  @Override
  @CanIgnoreReturnValue
  public boolean addAll(Collection<? extends E> collection) {
    int size = collection.size();
    if (size >= maxSize) {
      clear();
      return Iterables.addAll(this, Iterables.skip(collection, size - maxSize));
    }
    return standardAddAll(collection);
  }

  @Override
  public boolean contains(Object object) {
    return delegate().contains(checkNotNull(object));
  }

  @Override
  @CanIgnoreReturnValue
  public boolean remove(Object object) {
    return delegate().remove(checkNotNull(object));
  }

  
  private static final long serialVersionUID = 0L;
}
