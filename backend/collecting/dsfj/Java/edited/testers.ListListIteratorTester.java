

package com.google.common.collect.testing.testers;

import static com.google.common.collect.testing.IteratorFeature.MODIFIABLE;
import static com.google.common.collect.testing.IteratorFeature.UNMODIFIABLE;
import static com.google.common.collect.testing.features.CollectionFeature.SUPPORTS_REMOVE;
import static com.google.common.collect.testing.features.ListFeature.SUPPORTS_ADD_WITH_INDEX;
import static com.google.common.collect.testing.features.ListFeature.SUPPORTS_SET;
import static com.google.common.collect.testing.testers.Platform.listListIteratorTesterNumIterations;
import static java.util.Collections.singleton;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.Helpers;
import com.google.common.collect.testing.IteratorFeature;
import com.google.common.collect.testing.ListIteratorTester;
import com.google.common.collect.testing.features.CollectionFeature;
import com.google.common.collect.testing.features.ListFeature;
import java.lang.reflect.Method;
import java.util.List;
import java.util.ListIterator;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import org.junit.Ignore;


@GwtCompatible(emulated = true)
@Ignore public class ListListIteratorTester<E> extends AbstractListTester<E> {
  @CollectionFeature.Require(absent = SUPPORTS_REMOVE)
  @ListFeature.Require(absent = {SUPPORTS_SET, SUPPORTS_ADD_WITH_INDEX})
  public void testListIterator_unmodifiable() {
    runListIteratorTest(UNMODIFIABLE);
  }

  
  @CollectionFeature.Require(SUPPORTS_REMOVE)
  @ListFeature.Require({SUPPORTS_SET, SUPPORTS_ADD_WITH_INDEX})
  public void testListIterator_fullyModifiable() {
    runListIteratorTest(MODIFIABLE);
  }

  private void runListIteratorTest(Set<IteratorFeature> features) {
    new ListIteratorTester<E>(
        listListIteratorTesterNumIterations(),
        singleton(e4()),
        features,
        Helpers.copyToList(getOrderedElements()),
        0) {
      @Override
      protected ListIterator<E> newTargetIterator() {
        resetCollection();
        return getList().listIterator();
      }

      @Override
      protected void verify(List<E> elements) {
        expectContents(elements);
      }
    }.test();
  }

  public void testListIterator_tooLow() {
    try {
      getList().listIterator(-1);
      fail();
    } catch (IndexOutOfBoundsException expected) {
    }
  }

  public void testListIterator_tooHigh() {
    try {
      getList().listIterator(getNumElements() + 1);
      fail();
    } catch (IndexOutOfBoundsException expected) {
    }
  }

  public void testListIterator_atSize() {
    getList().listIterator(getNumElements());
      }

  
  @GwtIncompatible   public static Method getListIteratorFullyModifiableMethod() {
    return Helpers.getMethod(ListListIteratorTester.class, "testListIterator_fullyModifiable");
  }

  
  @GwtIncompatible   public static Method getListIteratorUnmodifiableMethod() {
    return Helpers.getMethod(ListListIteratorTester.class, "testListIterator_unmodifiable");
  }
}
