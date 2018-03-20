

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.Collections;
import java.util.Iterator;
import java.util.NoSuchElementException;
import junit.framework.TestCase;


@GwtCompatible
public class MinimalIterableTest extends TestCase {

  public void testOf_empty() {
    Iterable<String> iterable = MinimalIterable.<String>of();
    Iterator<String> iterator = iterable.iterator();
    assertFalse(iterator.hasNext());
    try {
      iterator.next();
      fail();
    } catch (NoSuchElementException expected) {
    }
    try {
      iterable.iterator();
      fail();
    } catch (IllegalStateException expected) {
    }
  }

  public void testOf_one() {
    Iterable<String> iterable = MinimalIterable.of("a");
    Iterator<String> iterator = iterable.iterator();
    assertTrue(iterator.hasNext());
    assertEquals("a", iterator.next());
    assertFalse(iterator.hasNext());
    try {
      iterator.next();
      fail();
    } catch (NoSuchElementException expected) {
    }
    try {
      iterable.iterator();
      fail();
    } catch (IllegalStateException expected) {
    }
  }

  public void testFrom_empty() {
    Iterable<String> iterable = MinimalIterable.from(Collections.<String>emptySet());
    Iterator<String> iterator = iterable.iterator();
    assertFalse(iterator.hasNext());
    try {
      iterator.next();
      fail();
    } catch (NoSuchElementException expected) {
    }
    try {
      iterable.iterator();
      fail();
    } catch (IllegalStateException expected) {
    }
  }

  public void testFrom_one() {
    Iterable<String> iterable = MinimalIterable.from(Collections.singleton("a"));
    Iterator<String> iterator = iterable.iterator();
    assertTrue(iterator.hasNext());
    assertEquals("a", iterator.next());
    try {
      iterator.remove();
      fail();
    } catch (UnsupportedOperationException expected) {
    }
    assertFalse(iterator.hasNext());
    try {
      iterator.next();
      fail();
    } catch (NoSuchElementException expected) {
    }
    try {
      iterable.iterator();
      fail();
    } catch (IllegalStateException expected) {
    }
  }
}
