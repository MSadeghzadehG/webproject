

package com.google.common.collect.testing;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.collect.testing.features.MapFeature.ALLOWS_NULL_KEYS;
import static com.google.common.collect.testing.features.MapFeature.ALLOWS_NULL_VALUES;

import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.testing.features.CollectionFeature;
import com.google.common.collect.testing.features.CollectionSize;
import com.google.common.collect.testing.features.Feature;
import com.google.common.collect.testing.features.MapFeature;
import java.util.AbstractMap;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;


public final class MapTestSuiteBuilderTests extends TestCase {
  private MapTestSuiteBuilderTests() {}

  public static Test suite() {
    TestSuite suite = new TestSuite(MapTestSuiteBuilderTests.class.getSimpleName());
    suite.addTest(testsForHashMapNullKeysForbidden());
    suite.addTest(testsForHashMapNullValuesForbidden());
    return suite;
  }

  private abstract static class WrappedHashMapGenerator extends TestStringMapGenerator {
    @Override
    protected final Map<String, String> create(Entry<String, String>[] entries) {
      HashMap<String, String> map = Maps.newHashMap();
      for (Entry<String, String> entry : entries) {
        map.put(entry.getKey(), entry.getValue());
      }
      return wrap(map);
    }

    abstract Map<String, String> wrap(HashMap<String, String> map);
  }

  private static TestSuite wrappedHashMapTests(
      WrappedHashMapGenerator generator, String name, Feature<?>... features) {
    List<Feature<?>> featuresList = Lists.newArrayList(features);
    Collections.addAll(
        featuresList,
        MapFeature.GENERAL_PURPOSE,
        CollectionFeature.SUPPORTS_ITERATOR_REMOVE,
        CollectionSize.ANY);
    return MapTestSuiteBuilder.using(generator)
        .named(name)
        .withFeatures(featuresList)
        .createTestSuite();
  }

  
  private static Test testsForHashMapNullKeysForbidden() {
    return wrappedHashMapTests(
        new WrappedHashMapGenerator() {
          @Override
          Map<String, String> wrap(final HashMap<String, String> map) {
            if (map.containsKey(null)) {
              throw new NullPointerException();
            }
            return new AbstractMap<String, String>() {
              @Override
              public Set<Entry<String, String>> entrySet() {
                return map.entrySet();
              }

              @Override
              public String put(String key, String value) {
                checkNotNull(key);
                return map.put(key, value);
              }
            };
          }
        },
        "HashMap w/out null keys",
        ALLOWS_NULL_VALUES);
  }

  private static Test testsForHashMapNullValuesForbidden() {
    return wrappedHashMapTests(
        new WrappedHashMapGenerator() {
          @Override
          Map<String, String> wrap(final HashMap<String, String> map) {
            if (map.containsValue(null)) {
              throw new NullPointerException();
            }
            return new AbstractMap<String, String>() {
              @Override
              public Set<Entry<String, String>> entrySet() {
                return map.entrySet();
              }

              @Override
              public String put(String key, String value) {
                checkNotNull(value);
                return map.put(key, value);
              }
            };
          }
        },
        "HashMap w/out null values",
        ALLOWS_NULL_KEYS);
  }
}
