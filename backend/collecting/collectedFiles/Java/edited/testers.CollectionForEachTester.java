

package com.google.common.collect.testing.testers;

import static com.google.common.collect.testing.features.CollectionFeature.KNOWN_ORDER;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.AbstractCollectionTester;
import com.google.common.collect.testing.Helpers;
import com.google.common.collect.testing.features.CollectionFeature;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import org.junit.Ignore;


@GwtCompatible
@Ignore public class CollectionForEachTester<E> extends AbstractCollectionTester<E> {
  @CollectionFeature.Require(absent = KNOWN_ORDER)
  public void testForEachUnknownOrder() {
    List<E> elements = new ArrayList<E>();
    collection.forEach(elements::add);
    Helpers.assertEqualIgnoringOrder(Arrays.asList(createSamplesArray()), elements);
  }

  @CollectionFeature.Require(KNOWN_ORDER)
  public void testForEachKnownOrder() {
    List<E> elements = new ArrayList<E>();
    collection.forEach(elements::add);
    List<E> expected = Helpers.copyToList(getOrderedElements());
    assertEquals("Different ordered iteration", expected, elements);
  }
}
