

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import java.util.Iterator;
import java.util.ListIterator;
import java.util.NoSuchElementException;
import junit.framework.TestCase;


@GwtCompatible
public class UnmodifiableListIteratorTest extends TestCase {
  public void testRemove() {
    Iterator<String> iterator = create();

    assertTrue(iterator.hasNext());
    assertEquals("a", iterator.next());
    try {
      iterator.remove();
      fail();
    } catch (UnsupportedOperationException expected) {
    }
  }

  public void testAdd() {
    ListIterator<String> iterator = create();

    assertTrue(iterator.hasNext());
    assertEquals("a", iterator.next());
    assertEquals("b", iterator.next());
    assertEquals("b", iterator.previous());
    try {
      iterator.add("c");
      fail();
    } catch (UnsupportedOperationException expected) {
    }
  }

  public void testSet() {
    ListIterator<String> iterator = create();

    assertTrue(iterator.hasNext());
    assertEquals("a", iterator.next());
    assertEquals("b", iterator.next());
    assertEquals("b", iterator.previous());
    try {
      iterator.set("c");
      fail();
    } catch (UnsupportedOperationException expected) {
    }
  }

  UnmodifiableListIterator<String> create() {
    final String[] array = {"a", "b", "c"};

    return new UnmodifiableListIterator<String>() {
      int i;

      @Override
      public boolean hasNext() {
        return i < array.length;
      }

      @Override
      public String next() {
        if (!hasNext()) {
          throw new NoSuchElementException();
        }
        return array[i++];
      }

      @Override
      public boolean hasPrevious() {
        return i > 0;
      }

      @Override
      public int nextIndex() {
        return i;
      }

      @Override
      public String previous() {
        if (!hasPrevious()) {
          throw new NoSuchElementException();
        }
        return array[--i];
      }

      @Override
      public int previousIndex() {
        return i - 1;
      }
    };
  }
}
