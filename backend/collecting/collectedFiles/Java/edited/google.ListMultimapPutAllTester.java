

package com.google.common.collect.testing.google;

import static com.google.common.collect.testing.Helpers.copyToList;
import static com.google.common.collect.testing.features.MapFeature.SUPPORTS_PUT;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.ListMultimap;
import com.google.common.collect.testing.features.MapFeature;
import java.util.Arrays;
import java.util.List;
import org.junit.Ignore;


@GwtCompatible
@Ignore public class ListMultimapPutAllTester<K, V> extends AbstractListMultimapTester<K, V> {
  @MapFeature.Require(SUPPORTS_PUT)
  public void testPutAllAddsAtEndInOrder() {
    @SuppressWarnings("unchecked")
    List<V> values = Arrays.asList(v3(), v1(), v4());

    for (K k : sampleKeys()) {
      resetContainer();

      List<V> expectedValues = copyToList(multimap().get(k));

      assertTrue(multimap().putAll(k, values));
      expectedValues.addAll(values);

      assertGet(k, expectedValues);
    }
  }
}
