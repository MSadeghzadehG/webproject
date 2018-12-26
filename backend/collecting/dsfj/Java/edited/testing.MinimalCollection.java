

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.AbstractCollection;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;


@GwtCompatible
public class MinimalCollection<E> extends AbstractCollection<E> {
  
  public static <E> MinimalCollection<E> of(E... contents) {
    return new MinimalCollection<E>(Object.class, true, contents);
  }

    public static <E> MinimalCollection<E> ofClassAndContents(Class<? super E> type, E... contents) {
    return new MinimalCollection<E>(type, true, contents);
  }

  private final E[] contents;
  private final Class<? super E> type;
  private final boolean allowNulls;

    MinimalCollection(Class<? super E> type, boolean allowNulls, E... contents) {
        this.contents = Platform.clone(contents);
    this.type = type;
    this.allowNulls = allowNulls;

    if (!allowNulls) {
      for (Object element : contents) {
        if (element == null) {
          throw new NullPointerException();
        }
      }
    }
  }

  @Override
  public int size() {
    return contents.length;
  }

  @Override
  public boolean contains(Object object) {
    if (!allowNulls) {
            if (object == null) {
        throw new NullPointerException();
      }
    }
    Platform.checkCast(type, object);     return Arrays.asList(contents).contains(object);
  }

  @Override
  public boolean containsAll(Collection<?> collection) {
    if (!allowNulls) {
      for (Object object : collection) {
                if (object == null) {
          throw new NullPointerException();
        }
      }
    }
    return super.containsAll(collection);
  }

  @Override
  public Iterator<E> iterator() {
    return Arrays.asList(contents).iterator();
  }

  @Override
  public Object[] toArray() {
    Object[] result = new Object[contents.length];
    System.arraycopy(contents, 0, result, 0, contents.length);
    return result;
  }

  

  @Override
  public boolean addAll(Collection<? extends E> elementsToAdd) {
    throw up();
  }

  @Override
  public boolean removeAll(Collection<?> elementsToRemove) {
    throw up();
  }

  @Override
  public boolean retainAll(Collection<?> elementsToRetain) {
    throw up();
  }

  @Override
  public void clear() {
    throw up();
  }

  private static UnsupportedOperationException up() {
    throw new UnsupportedOperationException();
  }
}
