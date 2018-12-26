

package com.google.common.collect.testing.google;

import static com.google.common.collect.testing.features.MapFeature.SUPPORTS_PUT;
import static com.google.common.collect.testing.features.MapFeature.SUPPORTS_REMOVE;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.ListMultimap;
import com.google.common.collect.testing.features.MapFeature;
import java.util.Arrays;
import java.util.List;
import org.junit.Ignore;


@GwtCompatible
@Ignore public class ListMultimapReplaceValuesTester<K, V> extends AbstractListMultimapTester<K, V> {
  @MapFeature.Require({SUPPORTS_PUT, SUPPORTS_REMOVE})
  public void testReplaceValuesPreservesOrder() {
    @SuppressWarnings("unchecked")
    List<V> values = Arrays.asList(v3(), v1(), v4());

    for (K k : sampleKeys()) {
      resetContainer();
      multimap().replaceValues(k, values);
      assertGet(k, values);
    }
  }
}
