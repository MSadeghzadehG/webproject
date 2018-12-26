

package com.google.common.collect.testing.testers;

import static com.google.common.collect.testing.features.CollectionFeature.NON_STANDARD_TOSTRING;
import static com.google.common.collect.testing.features.CollectionSize.ONE;
import static com.google.common.collect.testing.features.CollectionSize.ZERO;
import static com.google.common.collect.testing.features.MapFeature.ALLOWS_NULL_KEYS;
import static com.google.common.collect.testing.features.MapFeature.ALLOWS_NULL_VALUES;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.AbstractMapTester;
import com.google.common.collect.testing.features.CollectionFeature;
import com.google.common.collect.testing.features.CollectionSize;
import com.google.common.collect.testing.features.MapFeature;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import org.junit.Ignore;


@GwtCompatible
@Ignore public class MapToStringTester<K, V> extends AbstractMapTester<K, V> {
  public void testToString_minimal() {
    assertNotNull("toString() should not return null", getMap().toString());
  }

  @CollectionSize.Require(ZERO)
  @CollectionFeature.Require(absent = NON_STANDARD_TOSTRING)
  public void testToString_size0() {
    assertEquals("emptyMap.toString should return {}", "{}", getMap().toString());
  }

  @CollectionSize.Require(ONE)
  @CollectionFeature.Require(absent = NON_STANDARD_TOSTRING)
  public void testToString_size1() {
    assertEquals("size1Map.toString should return {entry}", "{" + e0() + "}", getMap().toString());
  }

  @CollectionSize.Require(absent = ZERO)
  @CollectionFeature.Require(absent = NON_STANDARD_TOSTRING)
  @MapFeature.Require(ALLOWS_NULL_KEYS)
  public void testToStringWithNullKey() {
    initMapWithNullKey();
    testToString_formatting();
  }

  @CollectionSize.Require(absent = ZERO)
  @CollectionFeature.Require(absent = NON_STANDARD_TOSTRING)
  @MapFeature.Require(ALLOWS_NULL_VALUES)
  public void testToStringWithNullValue() {
    initMapWithNullValue();
    testToString_formatting();
  }

  @CollectionFeature.Require(absent = NON_STANDARD_TOSTRING)
  public void testToString_formatting() {
    assertEquals(
        "map.toString() incorrect", expectedToString(getMap().entrySet()), getMap().toString());
  }

  private String expectedToString(Set<Entry<K, V>> entries) {
    Map<K, V> reference = new LinkedHashMap<>();
    for (Entry<K, V> entry : entries) {
      reference.put(entry.getKey(), entry.getValue());
    }
    return reference.toString();
  }
}
