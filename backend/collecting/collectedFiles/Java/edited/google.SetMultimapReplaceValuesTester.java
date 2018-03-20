

package com.google.common.collect.testing.google;

import static com.google.common.collect.testing.features.MapFeature.SUPPORTS_PUT;
import static com.google.common.collect.testing.features.MapFeature.SUPPORTS_REMOVE;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.SetMultimap;
import com.google.common.collect.testing.features.MapFeature;
import java.util.Arrays;
import java.util.List;
import org.junit.Ignore;


@GwtCompatible
@Ignore public class SetMultimapReplaceValuesTester<K, V>
    extends AbstractMultimapTester<K, V, SetMultimap<K, V>> {

  @MapFeature.Require({SUPPORTS_PUT, SUPPORTS_REMOVE})
  public void testReplaceValuesHandlesDuplicates() {
    @SuppressWarnings("unchecked")
    List<V> values = Arrays.asList(v0(), v1(), v0());

    for (K k : sampleKeys()) {
      resetContainer();
      multimap().replaceValues(k, values);
      assertGet(k, v0(), v1());
    }
  }
}
