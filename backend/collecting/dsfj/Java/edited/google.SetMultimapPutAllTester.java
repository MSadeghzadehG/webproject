

package com.google.common.collect.testing.google;

import static com.google.common.collect.testing.Helpers.copyToSet;
import static com.google.common.collect.testing.features.MapFeature.SUPPORTS_PUT;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.SetMultimap;
import com.google.common.collect.testing.features.MapFeature;
import java.util.Arrays;
import java.util.List;
import java.util.Set;
import org.junit.Ignore;


@GwtCompatible
@Ignore public class SetMultimapPutAllTester<K, V> extends AbstractMultimapTester<K, V, SetMultimap<K, V>> {

  @MapFeature.Require(SUPPORTS_PUT)
  public void testPutAllHandlesDuplicates() {
    @SuppressWarnings("unchecked")
    List<V> valuesToPut = Arrays.asList(v0(), v1(), v0());

    for (K k : sampleKeys()) {
      resetContainer();

      Set<V> expectedValues = copyToSet(multimap().get(k));

      multimap().putAll(k, valuesToPut);
      expectedValues.addAll(valuesToPut);

      assertGet(k, expectedValues);
    }
  }
}
