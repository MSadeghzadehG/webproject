

package com.google.common.primitives;

import static com.google.common.base.Preconditions.checkArgument;
import static com.google.common.base.Preconditions.checkElementIndex;
import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.base.Preconditions.checkPositionIndexes;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import java.io.Serializable;
import java.util.AbstractList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.RandomAccess;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
public final class Booleans {
  private Booleans() {}

  
  private enum BooleanComparator implements Comparator<Boolean> {
    TRUE_FIRST(1, "Booleans.trueFirst()"),
    FALSE_FIRST(-1, "Booleans.falseFirst()");

    private final int trueValue;
    private final String toString;

    BooleanComparator(int trueValue, String toString) {
      this.trueValue = trueValue;
      this.toString = toString;
    }

    @Override
    public int compare(Boolean a, Boolean b) {
      int aVal = a ? trueValue : 0;
      int bVal = b ? trueValue : 0;
      return bVal - aVal;
    }

    @Override
    public String toString() {
      return toString;
    }
  }

  
  @Beta
  public static Comparator<Boolean> trueFirst() {
    return BooleanComparator.TRUE_FIRST;
  }

  
  @Beta
  public static Comparator<Boolean> falseFirst() {
    return BooleanComparator.FALSE_FIRST;
  }

  
  public static int hashCode(boolean value) {
    return value ? 1231 : 1237;
  }

  
  public static int compare(boolean a, boolean b) {
    return (a == b) ? 0 : (a ? 1 : -1);
  }

  
  public static boolean contains(boolean[] array, boolean target) {
    for (boolean value : array) {
      if (value == target) {
        return true;
      }
    }
    return false;
  }

  
  public static int indexOf(boolean[] array, boolean target) {
    return indexOf(array, target, 0, array.length);
  }

    private static int indexOf(boolean[] array, boolean target, int start, int end) {
    for (int i = start; i < end; i++) {
      if (array[i] == target) {
        return i;
      }
    }
    return -1;
  }

  
  public static int indexOf(boolean[] array, boolean[] target) {
    checkNotNull(array, "array");
    checkNotNull(target, "target");
    if (target.length == 0) {
      return 0;
    }

    outer:
    for (int i = 0; i < array.length - target.length + 1; i++) {
      for (int j = 0; j < target.length; j++) {
        if (array[i + j] != target[j]) {
          continue outer;
        }
      }
      return i;
    }
    return -1;
  }

  
  public static int lastIndexOf(boolean[] array, boolean target) {
    return lastIndexOf(array, target, 0, array.length);
  }

    private static int lastIndexOf(boolean[] array, boolean target, int start, int end) {
    for (int i = end - 1; i >= start; i--) {
      if (array[i] == target) {
        return i;
      }
    }
    return -1;
  }

  
  public static boolean[] concat(boolean[]... arrays) {
    int length = 0;
    for (boolean[] array : arrays) {
      length += array.length;
    }
    boolean[] result = new boolean[length];
    int pos = 0;
    for (boolean[] array : arrays) {
      System.arraycopy(array, 0, result, pos, array.length);
      pos += array.length;
    }
    return result;
  }

  
  public static boolean[] ensureCapacity(boolean[] array, int minLength, int padding) {
    checkArgument(minLength >= 0, "Invalid minLength: %s", minLength);
    checkArgument(padding >= 0, "Invalid padding: %s", padding);
    return (array.length < minLength) ? Arrays.copyOf(array, minLength + padding) : array;
  }

  
  public static String join(String separator, boolean... array) {
    checkNotNull(separator);
    if (array.length == 0) {
      return "";
    }

        StringBuilder builder = new StringBuilder(array.length * 7);
    builder.append(array[0]);
    for (int i = 1; i < array.length; i++) {
      builder.append(separator).append(array[i]);
    }
    return builder.toString();
  }

  
  public static Comparator<boolean[]> lexicographicalComparator() {
    return LexicographicalComparator.INSTANCE;
  }

  private enum LexicographicalComparator implements Comparator<boolean[]> {
    INSTANCE;

    @Override
    public int compare(boolean[] left, boolean[] right) {
      int minLength = Math.min(left.length, right.length);
      for (int i = 0; i < minLength; i++) {
        int result = Booleans.compare(left[i], right[i]);
        if (result != 0) {
          return result;
        }
      }
      return left.length - right.length;
    }

    @Override
    public String toString() {
      return "Booleans.lexicographicalComparator()";
    }
  }

  
  public static boolean[] toArray(Collection<Boolean> collection) {
    if (collection instanceof BooleanArrayAsList) {
      return ((BooleanArrayAsList) collection).toBooleanArray();
    }

    Object[] boxedArray = collection.toArray();
    int len = boxedArray.length;
    boolean[] array = new boolean[len];
    for (int i = 0; i < len; i++) {
            array[i] = (Boolean) checkNotNull(boxedArray[i]);
    }
    return array;
  }

  
  public static List<Boolean> asList(boolean... backingArray) {
    if (backingArray.length == 0) {
      return Collections.emptyList();
    }
    return new BooleanArrayAsList(backingArray);
  }

  @GwtCompatible
  private static class BooleanArrayAsList extends AbstractList<Boolean>
      implements RandomAccess, Serializable {
    final boolean[] array;
    final int start;
    final int end;

    BooleanArrayAsList(boolean[] array) {
      this(array, 0, array.length);
    }

    BooleanArrayAsList(boolean[] array, int start, int end) {
      this.array = array;
      this.start = start;
      this.end = end;
    }

    @Override
    public int size() {
      return end - start;
    }

    @Override
    public boolean isEmpty() {
      return false;
    }

    @Override
    public Boolean get(int index) {
      checkElementIndex(index, size());
      return array[start + index];
    }

    @Override
    public boolean contains(Object target) {
            return (target instanceof Boolean)
          && Booleans.indexOf(array, (Boolean) target, start, end) != -1;
    }

    @Override
    public int indexOf(Object target) {
            if (target instanceof Boolean) {
        int i = Booleans.indexOf(array, (Boolean) target, start, end);
        if (i >= 0) {
          return i - start;
        }
      }
      return -1;
    }

    @Override
    public int lastIndexOf(Object target) {
            if (target instanceof Boolean) {
        int i = Booleans.lastIndexOf(array, (Boolean) target, start, end);
        if (i >= 0) {
          return i - start;
        }
      }
      return -1;
    }

    @Override
    public Boolean set(int index, Boolean element) {
      checkElementIndex(index, size());
      boolean oldValue = array[start + index];
            array[start + index] = checkNotNull(element);
      return oldValue;
    }

    @Override
    public List<Boolean> subList(int fromIndex, int toIndex) {
      int size = size();
      checkPositionIndexes(fromIndex, toIndex, size);
      if (fromIndex == toIndex) {
        return Collections.emptyList();
      }
      return new BooleanArrayAsList(array, start + fromIndex, start + toIndex);
    }

    @Override
    public boolean equals(@NullableDecl Object object) {
      if (object == this) {
        return true;
      }
      if (object instanceof BooleanArrayAsList) {
        BooleanArrayAsList that = (BooleanArrayAsList) object;
        int size = size();
        if (that.size() != size) {
          return false;
        }
        for (int i = 0; i < size; i++) {
          if (array[start + i] != that.array[that.start + i]) {
            return false;
          }
        }
        return true;
      }
      return super.equals(object);
    }

    @Override
    public int hashCode() {
      int result = 1;
      for (int i = start; i < end; i++) {
        result = 31 * result + Booleans.hashCode(array[i]);
      }
      return result;
    }

    @Override
    public String toString() {
      StringBuilder builder = new StringBuilder(size() * 7);
      builder.append(array[start] ? "[true" : "[false");
      for (int i = start + 1; i < end; i++) {
        builder.append(array[i] ? ", true" : ", false");
      }
      return builder.append(']').toString();
    }

    boolean[] toBooleanArray() {
      return Arrays.copyOfRange(array, start, end);
    }

    private static final long serialVersionUID = 0;
  }

  
  @Beta
  public static int countTrue(boolean... values) {
    int count = 0;
    for (boolean value : values) {
      if (value) {
        count++;
      }
    }
    return count;
  }

  
  public static void reverse(boolean[] array) {
    checkNotNull(array);
    reverse(array, 0, array.length);
  }

  
  public static void reverse(boolean[] array, int fromIndex, int toIndex) {
    checkNotNull(array);
    checkPositionIndexes(fromIndex, toIndex, array.length);
    for (int i = fromIndex, j = toIndex - 1; i < j; i++, j--) {
      boolean tmp = array[i];
      array[i] = array[j];
      array[j] = tmp;
    }
  }
}
