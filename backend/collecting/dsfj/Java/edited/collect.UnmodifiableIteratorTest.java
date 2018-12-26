

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import java.util.Iterator;
import java.util.NoSuchElementException;
import junit.framework.TestCase;


@GwtCompatible
public class UnmodifiableIteratorTest extends TestCase {

  public void testRemove() {
    final String[] array = {"a", "b", "c"};

    Iterator<String> iterator =
        new UnmodifiableIterator<String>() {
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
        };

    assertTrue(iterator.hasNext());
    assertEquals("a", iterator.next());
    try {
      iterator.remove();
      fail();
    } catch (UnsupportedOperationException expected) {
    }
  }
}
