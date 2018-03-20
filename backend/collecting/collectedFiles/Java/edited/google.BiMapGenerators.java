

package com.google.common.collect.testing.google;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.BiMap;
import com.google.common.collect.ImmutableBiMap;
import com.google.common.collect.Maps;
import java.util.Arrays;
import java.util.Map;
import java.util.Map.Entry;


@GwtCompatible
public class BiMapGenerators {
  public static class ImmutableBiMapGenerator extends TestStringBiMapGenerator {
    @Override
    protected BiMap<String, String> create(Entry<String, String>[] entries) {
      ImmutableBiMap.Builder<String, String> builder = ImmutableBiMap.builder();
      for (Entry<String, String> entry : entries) {
        builder.put(entry.getKey(), entry.getValue());
      }
      return builder.build();
    }
  }

  public static class ImmutableBiMapCopyOfGenerator extends TestStringBiMapGenerator {
    @Override
    protected BiMap<String, String> create(Entry<String, String>[] entries) {
      Map<String, String> builder = Maps.newLinkedHashMap();
      for (Entry<String, String> entry : entries) {
        builder.put(entry.getKey(), entry.getValue());
      }
      return ImmutableBiMap.copyOf(builder);
    }
  }

  public static class ImmutableBiMapCopyOfEntriesGenerator extends TestStringBiMapGenerator {
    @Override
    protected BiMap<String, String> create(Entry<String, String>[] entries) {
      return ImmutableBiMap.copyOf(Arrays.asList(entries));
    }
  }
}
