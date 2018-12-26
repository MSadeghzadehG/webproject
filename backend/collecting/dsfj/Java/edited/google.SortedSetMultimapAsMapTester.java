

package com.google.common.collect.testing.google;

import static com.google.common.collect.testing.features.MapFeature.SUPPORTS_REMOVE;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.SortedSetMultimap;
import com.google.common.collect.testing.features.MapFeature;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.SortedSet;
import org.junit.Ignore;


@GwtCompatible
@Ignore public class SortedSetMultimapAsMapTester<K, V>
    extends AbstractMultimapTester<K, V, SortedSetMultimap<K, V>> {
  public void testAsMapValuesImplementSortedSet() {
    for (Collection<V> valueCollection : multimap().asMap().values()) {
      SortedSet<V> valueSet = (SortedSet<V>) valueCollection;
      assertEquals(multimap().valueComparator(), valueSet.comparator());
    }
  }

  public void testAsMapGetImplementsSortedSet() {
    for (K key : multimap().keySet()) {
      SortedSet<V> valueSet = (SortedSet<V>) multimap().asMap().get(key);
      assertEquals(multimap().valueComparator(), valueSet.comparator());
    }
  }

  @MapFeature.Require(SUPPORTS_REMOVE)
  public void testAsMapRemoveImplementsSortedSet() {
    List<K> keys = new ArrayList<K>(multimap().keySet());
    for (K key : keys) {
      resetCollection();
      SortedSet<V> valueSet = (SortedSet<V>) multimap().asMap().remove(key);
      assertEquals(multimap().valueComparator(), valueSet.comparator());
    }
  }
}
