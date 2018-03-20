

package com.google.common.collect;

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.collect.CollectPreconditions.checkNonnegative;
import static com.google.common.collect.CollectPreconditions.checkRemove;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.primitives.Ints;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.Arrays;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.function.ObjIntConsumer;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible(emulated = true)
public final class EnumMultiset<E extends Enum<E>> extends AbstractMultiset<E>
    implements Serializable {
  
  public static <E extends Enum<E>> EnumMultiset<E> create(Class<E> type) {
    return new EnumMultiset<E>(type);
  }

  
  public static <E extends Enum<E>> EnumMultiset<E> create(Iterable<E> elements) {
    Iterator<E> iterator = elements.iterator();
    checkArgument(iterator.hasNext(), "EnumMultiset constructor passed empty Iterable");
    EnumMultiset<E> multiset = new EnumMultiset<>(iterator.next().getDeclaringClass());
    Iterables.addAll(multiset, elements);
    return multiset;
  }

  
  public static <E extends Enum<E>> EnumMultiset<E> create(Iterable<E> elements, Class<E> type) {
    EnumMultiset<E> result = create(type);
    Iterables.addAll(result, elements);
    return result;
  }

  private transient Class<E> type;
  private transient E[] enumConstants;
  private transient int[] counts;
  private transient int distinctElements;
  private transient long size;

  
  private EnumMultiset(Class<E> type) {
    this.type = type;
    checkArgument(type.isEnum());
    this.enumConstants = type.getEnumConstants();
    this.counts = new int[enumConstants.length];
  }

  private boolean isActuallyE(@NullableDecl Object o) {
    if (o instanceof Enum) {
      Enum<?> e = (Enum<?>) o;
      int index = e.ordinal();
      return index < enumConstants.length && enumConstants[index] == e;
    }
    return false;
  }

  
  @SuppressWarnings("unchecked")
  void checkIsE(@NullableDecl Object element) {
    checkNotNull(element);
    if (!isActuallyE(element)) {
      throw new ClassCastException("Expected an " + type + " but got " + element);
    }
  }

  @Override
  int distinctElements() {
    return distinctElements;
  }

  @Override
  public int size() {
    return Ints.saturatedCast(size);
  }

  @Override
  public int count(@NullableDecl Object element) {
    if (element == null || !isActuallyE(element)) {
      return 0;
    }
    Enum<?> e = (Enum<?>) element;
    return counts[e.ordinal()];
  }

    @CanIgnoreReturnValue
  @Override
  public int add(E element, int occurrences) {
    checkIsE(element);
    checkNonnegative(occurrences, "occurrences");
    if (occurrences == 0) {
      return count(element);
    }
    int index = element.ordinal();
    int oldCount = counts[index];
    long newCount = (long) oldCount + occurrences;
    checkArgument(newCount <= Integer.MAX_VALUE, "too many occurrences: %s", newCount);
    counts[index] = (int) newCount;
    if (oldCount == 0) {
      distinctElements++;
    }
    size += occurrences;
    return oldCount;
  }

    @CanIgnoreReturnValue
  @Override
  public int remove(@NullableDecl Object element, int occurrences) {
    if (element == null || !isActuallyE(element)) {
      return 0;
    }
    Enum<?> e = (Enum<?>) element;
    checkNonnegative(occurrences, "occurrences");
    if (occurrences == 0) {
      return count(element);
    }
    int index = e.ordinal();
    int oldCount = counts[index];
    if (oldCount == 0) {
      return 0;
    } else if (oldCount <= occurrences) {
      counts[index] = 0;
      distinctElements--;
      size -= oldCount;
    } else {
      counts[index] = oldCount - occurrences;
      size -= occurrences;
    }
    return oldCount;
  }

    @CanIgnoreReturnValue
  @Override
  public int setCount(E element, int count) {
    checkIsE(element);
    checkNonnegative(count, "count");
    int index = element.ordinal();
    int oldCount = counts[index];
    counts[index] = count;
    size += count - oldCount;
    if (oldCount == 0 && count > 0) {
      distinctElements++;
    } else if (oldCount > 0 && count == 0) {
      distinctElements--;
    }
    return oldCount;
  }

  @Override
  public void clear() {
    Arrays.fill(counts, 0);
    size = 0;
    distinctElements = 0;
  }

  abstract class Itr<T> implements Iterator<T> {
    int index = 0;
    int toRemove = -1;

    abstract T output(int index);

    @Override
    public boolean hasNext() {
      for (; index < enumConstants.length; index++) {
        if (counts[index] > 0) {
          return true;
        }
      }
      return false;
    }

    @Override
    public T next() {
      if (!hasNext()) {
        throw new NoSuchElementException();
      }
      T result = output(index);
      toRemove = index;
      index++;
      return result;
    }

    @Override
    public void remove() {
      checkRemove(toRemove >= 0);
      if (counts[toRemove] > 0) {
        distinctElements--;
        size -= counts[toRemove];
        counts[toRemove] = 0;
      }
      toRemove = -1;
    }
  }

  @Override
  Iterator<E> elementIterator() {
    return new Itr<E>() {
      @Override
      E output(int index) {
        return enumConstants[index];
      }
    };
  }

  @Override
  Iterator<Entry<E>> entryIterator() {
    return new Itr<Entry<E>>() {
      @Override
      Entry<E> output(final int index) {
        return new Multisets.AbstractEntry<E>() {
          @Override
          public E getElement() {
            return enumConstants[index];
          }

          @Override
          public int getCount() {
            return counts[index];
          }
        };
      }
    };
  }

  @Override
  public void forEachEntry(ObjIntConsumer<? super E> action) {
    checkNotNull(action);
    for (int i = 0; i < enumConstants.length; i++) {
      if (counts[i] > 0) {
        action.accept(enumConstants[i], counts[i]);
      }
    }
  }

  @Override
  public Iterator<E> iterator() {
    return Multisets.iteratorImpl(this);
  }

  @GwtIncompatible   private void writeObject(ObjectOutputStream stream) throws IOException {
    stream.defaultWriteObject();
    stream.writeObject(type);
    Serialization.writeMultiset(this, stream);
  }

  
  @GwtIncompatible   private void readObject(ObjectInputStream stream) throws IOException, ClassNotFoundException {
    stream.defaultReadObject();
    @SuppressWarnings("unchecked")     Class<E> localType = (Class<E>) stream.readObject();
    type = localType;
    enumConstants = type.getEnumConstants();
    counts = new int[enumConstants.length];
    Serialization.populateMultiset(this, stream);
  }

  @GwtIncompatible   private static final long serialVersionUID = 0;
}
