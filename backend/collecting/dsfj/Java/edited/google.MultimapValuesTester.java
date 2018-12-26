

package com.google.common.collect.testing.google;

import static com.google.common.collect.testing.Helpers.assertEqualIgnoringOrder;
import static com.google.common.collect.testing.Helpers.assertEqualInOrder;
import static com.google.common.collect.testing.features.CollectionFeature.KNOWN_ORDER;
import static com.google.common.collect.testing.features.CollectionFeature.SUPPORTS_ITERATOR_REMOVE;
import static com.google.common.collect.testing.features.CollectionSize.ONE;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.Lists;
import com.google.common.collect.Multimap;
import com.google.common.collect.testing.features.CollectionFeature;
import com.google.common.collect.testing.features.CollectionSize;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;
import org.junit.Ignore;


@GwtCompatible
@Ignore public class MultimapValuesTester<K, V> extends AbstractMultimapTester<K, V, Multimap<K, V>> {
  public void testValues() {
    List<V> expected = Lists.newArrayList();
    for (Entry<K, V> entry : getSampleElements()) {
      expected.add(entry.getValue());
    }
    assertEqualIgnoringOrder(expected, multimap().values());
  }

  @CollectionFeature.Require(KNOWN_ORDER)
  public void testValuesInOrder() {
    List<V> expected = Lists.newArrayList();
    for (Entry<K, V> entry : getOrderedElements()) {
      expected.add(entry.getValue());
    }
    assertEqualInOrder(expected, multimap().values());
  }

  @CollectionFeature.Require(SUPPORTS_ITERATOR_REMOVE)
  @CollectionSize.Require(ONE)
  public void testValuesIteratorRemove() {
    Iterator<V> valuesItr = multimap().values().iterator();
    valuesItr.next();
    valuesItr.remove();
    assertTrue(multimap().isEmpty());
  }
}
