

package com.google.common.collect.testing.google;

import static com.google.common.collect.testing.features.CollectionFeature.KNOWN_ORDER;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.Multiset.Entry;
import com.google.common.collect.Multisets;
import com.google.common.collect.testing.Helpers;
import com.google.common.collect.testing.features.CollectionFeature;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import org.junit.Ignore;


@GwtCompatible(emulated = true)
@Ignore public class MultisetForEachEntryTester<E> extends AbstractMultisetTester<E> {
  public void testForEachEntry() {
    List<Entry<E>> expected = new ArrayList<>(getMultiset().entrySet());
    List<Entry<E>> actual = new ArrayList<>();
    getMultiset()
        .forEachEntry((element, count) -> actual.add(Multisets.immutableEntry(element, count)));
    Helpers.assertEqualIgnoringOrder(expected, actual);
  }

  @CollectionFeature.Require(KNOWN_ORDER)
  public void testForEachEntryOrdered() {
    List<Entry<E>> expected = new ArrayList<>(getMultiset().entrySet());
    List<Entry<E>> actual = new ArrayList<>();
    getMultiset()
        .forEachEntry((element, count) -> actual.add(Multisets.immutableEntry(element, count)));
    assertEquals(expected, actual);
  }

  public void testForEachEntryDuplicates() {
    initThreeCopies();
    List<Entry<E>> expected = Collections.singletonList(Multisets.immutableEntry(e0(), 3));
    List<Entry<E>> actual = new ArrayList<>();
    getMultiset()
        .forEachEntry((element, count) -> actual.add(Multisets.immutableEntry(element, count)));
    assertEquals(expected, actual);
  }

  
  @GwtIncompatible   public static List<Method> getForEachEntryDuplicateInitializingMethods() {
    return Arrays.asList(
        Helpers.getMethod(MultisetForEachEntryTester.class, "testForEachEntryDuplicates"));
  }
}
