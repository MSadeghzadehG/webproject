

package com.google.common.collect.testing.google;

import static com.google.common.collect.testing.IteratorFeature.MODIFIABLE;
import static com.google.common.collect.testing.IteratorFeature.UNMODIFIABLE;
import static com.google.common.collect.testing.features.CollectionFeature.KNOWN_ORDER;
import static com.google.common.collect.testing.features.CollectionFeature.SUPPORTS_ITERATOR_REMOVE;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.Helpers;
import com.google.common.collect.testing.IteratorTester;
import com.google.common.collect.testing.features.CollectionFeature;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import org.junit.Ignore;


@GwtCompatible(emulated = true)
@Ignore public class MultisetIteratorTester<E> extends AbstractMultisetTester<E> {
  @SuppressWarnings("unchecked")
  @CollectionFeature.Require({SUPPORTS_ITERATOR_REMOVE, KNOWN_ORDER})
  public void testRemovingIteratorKnownOrder() {
    new IteratorTester<E>(
        4,
        MODIFIABLE,
        getSubjectGenerator().order(Arrays.asList(e0(), e1(), e1(), e2())),
        IteratorTester.KnownOrder.KNOWN_ORDER) {
      @Override
      protected Iterator<E> newTargetIterator() {
        return getSubjectGenerator().create(e0(), e1(), e1(), e2()).iterator();
      }
    }.test();
  }

  @SuppressWarnings("unchecked")
  @CollectionFeature.Require(value = SUPPORTS_ITERATOR_REMOVE, absent = KNOWN_ORDER)
  public void testRemovingIteratorUnknownOrder() {
    new IteratorTester<E>(
        4,
        MODIFIABLE,
        Arrays.asList(e0(), e1(), e1(), e2()),
        IteratorTester.KnownOrder.UNKNOWN_ORDER) {
      @Override
      protected Iterator<E> newTargetIterator() {
        return getSubjectGenerator().create(e0(), e1(), e1(), e2()).iterator();
      }
    }.test();
  }

  @SuppressWarnings("unchecked")
  @CollectionFeature.Require(value = KNOWN_ORDER, absent = SUPPORTS_ITERATOR_REMOVE)
  public void testIteratorKnownOrder() {
    new IteratorTester<E>(
        4,
        UNMODIFIABLE,
        getSubjectGenerator().order(Arrays.asList(e0(), e1(), e1(), e2())),
        IteratorTester.KnownOrder.KNOWN_ORDER) {
      @Override
      protected Iterator<E> newTargetIterator() {
        return getSubjectGenerator().create(e0(), e1(), e1(), e2()).iterator();
      }
    }.test();
  }

  @SuppressWarnings("unchecked")
  @CollectionFeature.Require(absent = {SUPPORTS_ITERATOR_REMOVE, KNOWN_ORDER})
  public void testIteratorUnknownOrder() {
    new IteratorTester<E>(
        4,
        UNMODIFIABLE,
        Arrays.asList(e0(), e1(), e1(), e2()),
        IteratorTester.KnownOrder.UNKNOWN_ORDER) {
      @Override
      protected Iterator<E> newTargetIterator() {
        return getSubjectGenerator().create(e0(), e1(), e1(), e2()).iterator();
      }
    }.test();
  }

  
  @GwtIncompatible   public static List<Method> getIteratorDuplicateInitializingMethods() {
    return Arrays.asList(
        Helpers.getMethod(MultisetIteratorTester.class, "testIteratorKnownOrder"),
        Helpers.getMethod(MultisetIteratorTester.class, "testIteratorUnknownOrder"),
        Helpers.getMethod(MultisetIteratorTester.class, "testRemovingIteratorKnownOrder"),
        Helpers.getMethod(MultisetIteratorTester.class, "testRemovingIteratorUnknownOrder"));
  }
}
