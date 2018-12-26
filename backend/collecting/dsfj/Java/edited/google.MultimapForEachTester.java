
package com.google.common.collect.testing.google;

import static com.google.common.collect.testing.Helpers.assertEqualIgnoringOrder;
import static com.google.common.collect.testing.Helpers.mapEntry;
import static com.google.common.collect.testing.features.CollectionFeature.KNOWN_ORDER;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.Multimap;
import com.google.common.collect.testing.features.CollectionFeature;
import java.util.ArrayList;
import java.util.List;
import java.util.Map.Entry;
import org.junit.Ignore;


@GwtCompatible
@Ignore public class MultimapForEachTester<K, V> extends AbstractMultimapTester<K, V, Multimap<K, V>> {
  public void testForEach() {
    List<Entry<K, V>> entries = new ArrayList<>();
    multimap().forEach((k, v) -> entries.add(mapEntry(k, v)));
    assertEqualIgnoringOrder(getSampleElements(), multimap().entries());
  }

  @CollectionFeature.Require(KNOWN_ORDER)
  public void testForEachOrder() {
    List<Entry<K, V>> entries = new ArrayList<>();
    multimap().forEach((k, v) -> entries.add(mapEntry(k, v)));
    assertEqualIgnoringOrder(getSampleElements(), multimap().entries());
  }
}
