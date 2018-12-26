

package com.google.common.collect.testing.google;

import static com.google.common.collect.testing.features.CollectionFeature.ALLOWS_NULL_QUERIES;
import static com.google.common.collect.testing.features.CollectionFeature.ALLOWS_NULL_VALUES;
import static com.google.common.collect.testing.features.CollectionSize.SEVERAL;
import static com.google.common.collect.testing.features.CollectionSize.ZERO;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.Helpers;
import com.google.common.collect.testing.WrongType;
import com.google.common.collect.testing.features.CollectionFeature;
import com.google.common.collect.testing.features.CollectionSize;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.List;
import org.junit.Ignore;


@GwtCompatible(emulated = true)
@Ignore public class MultisetCountTester<E> extends AbstractMultisetTester<E> {

  public void testCount_0() {
    assertEquals("multiset.count(missing) didn't return 0", 0, getMultiset().count(e3()));
  }

  @CollectionSize.Require(absent = ZERO)
  public void testCount_1() {
    assertEquals("multiset.count(present) didn't return 1", 1, getMultiset().count(e0()));
  }

  @CollectionSize.Require(SEVERAL)
  public void testCount_3() {
    initThreeCopies();
    assertEquals("multiset.count(thriceContained) didn't return 3", 3, getMultiset().count(e0()));
  }

  @CollectionFeature.Require(ALLOWS_NULL_QUERIES)
  public void testCount_nullAbsent() {
    assertEquals("multiset.count(null) didn't return 0", 0, getMultiset().count(null));
  }

  @CollectionFeature.Require(absent = ALLOWS_NULL_QUERIES)
  public void testCount_null_forbidden() {
    try {
      getMultiset().count(null);
      fail("Expected NullPointerException");
    } catch (NullPointerException expected) {
    }
  }

  @CollectionSize.Require(absent = ZERO)
  @CollectionFeature.Require(ALLOWS_NULL_VALUES)
  public void testCount_nullPresent() {
    initCollectionWithNullElement();
    assertEquals(1, getMultiset().count(null));
  }

  public void testCount_wrongType() {
    assertEquals(
        "multiset.count(wrongType) didn't return 0", 0, getMultiset().count(WrongType.VALUE));
  }

  
  @GwtIncompatible   public static List<Method> getCountDuplicateInitializingMethods() {
    return Arrays.asList(Helpers.getMethod(MultisetCountTester.class, "testCount_3"));
  }
}
