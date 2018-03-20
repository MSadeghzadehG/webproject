

package com.google.common.collect.testing.testers;

import static com.google.common.collect.testing.features.CollectionSize.ONE;
import static com.google.common.collect.testing.features.CollectionSize.SEVERAL;
import static com.google.common.collect.testing.features.CollectionSize.ZERO;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.Helpers;
import com.google.common.collect.testing.features.CollectionSize;
import java.util.Collections;
import java.util.List;
import java.util.NoSuchElementException;
import java.util.SortedSet;
import org.junit.Ignore;


@GwtCompatible
@Ignore public class SortedSetNavigationTester<E> extends AbstractSetTester<E> {

  private SortedSet<E> sortedSet;
  private List<E> values;
  private E a;
  private E b;
  private E c;

  @Override
  public void setUp() throws Exception {
    super.setUp();
    sortedSet = (SortedSet<E>) getSet();
    values =
        Helpers.copyToList(
            getSubjectGenerator()
                .getSampleElements(getSubjectGenerator().getCollectionSize().getNumElements()));
    Collections.sort(values, sortedSet.comparator());

        if (values.size() >= 1) {
      a = values.get(0);
      if (values.size() >= 3) {
        b = values.get(1);
        c = values.get(2);
      }
    }
  }

  @CollectionSize.Require(ZERO)
  public void testEmptySetFirst() {
    try {
      sortedSet.first();
      fail();
    } catch (NoSuchElementException e) {
    }
  }

  @CollectionSize.Require(ZERO)
  public void testEmptySetLast() {
    try {
      sortedSet.last();
      fail();
    } catch (NoSuchElementException e) {
    }
  }

  @CollectionSize.Require(ONE)
  public void testSingletonSetFirst() {
    assertEquals(a, sortedSet.first());
  }

  @CollectionSize.Require(ONE)
  public void testSingletonSetLast() {
    assertEquals(a, sortedSet.last());
  }

  @CollectionSize.Require(SEVERAL)
  public void testFirst() {
    assertEquals(a, sortedSet.first());
  }

  @CollectionSize.Require(SEVERAL)
  public void testLast() {
    assertEquals(c, sortedSet.last());
  }
}
