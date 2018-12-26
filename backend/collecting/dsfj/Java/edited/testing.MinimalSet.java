

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Set;


@GwtCompatible
public class MinimalSet<E> extends MinimalCollection<E> implements Set<E> {

  @SuppressWarnings("unchecked")   public static <E> MinimalSet<E> of(E... contents) {
    return ofClassAndContents(Object.class, (E[]) new Object[0], Arrays.asList(contents));
  }

  @SuppressWarnings("unchecked")   public static <E> MinimalSet<E> from(Collection<? extends E> contents) {
    return ofClassAndContents(Object.class, (E[]) new Object[0], contents);
  }

  public static <E> MinimalSet<E> ofClassAndContents(
      Class<? super E> type, E[] emptyArrayForContents, Iterable<? extends E> contents) {
    List<E> setContents = new ArrayList<E>();
    for (E e : contents) {
      if (!setContents.contains(e)) {
        setContents.add(e);
      }
    }
    return new MinimalSet<E>(type, setContents.toArray(emptyArrayForContents));
  }

  private MinimalSet(Class<? super E> type, E... contents) {
    super(type, true, contents);
  }

  

  @Override
  public boolean equals(Object object) {
    if (object instanceof Set) {
      Set<?> that = (Set<?>) object;
      return (this.size() == that.size()) && this.containsAll(that);
    }
    return false;
  }

  @Override
  public int hashCode() {
    int hashCodeSum = 0;
    for (Object o : this) {
      hashCodeSum += (o == null) ? 0 : o.hashCode();
    }
    return hashCodeSum;
  }
}
