

package com.google.common.collect.testing;

import static com.google.common.collect.testing.Helpers.orderEntriesByKey;

import com.google.common.annotations.GwtCompatible;
import java.util.List;
import java.util.Map.Entry;
import java.util.SortedMap;


@GwtCompatible
public abstract class TestStringSortedMapGenerator extends TestStringMapGenerator
    implements TestSortedMapGenerator<String, String> {
  @Override
  public Entry<String, String> belowSamplesLesser() {
    return Helpers.mapEntry("!! a", "below view");
  }

  @Override
  public Entry<String, String> belowSamplesGreater() {
    return Helpers.mapEntry("!! b", "below view");
  }

  @Override
  public Entry<String, String> aboveSamplesLesser() {
    return Helpers.mapEntry("~~ a", "above view");
  }

  @Override
  public Entry<String, String> aboveSamplesGreater() {
    return Helpers.mapEntry("~~ b", "above view");
  }

  @Override
  public Iterable<Entry<String, String>> order(List<Entry<String, String>> insertionOrder) {
    return orderEntriesByKey(insertionOrder);
  }

  @Override
  protected abstract SortedMap<String, String> create(Entry<String, String>[] entries);

  @Override
  public SortedMap<String, String> create(Object... entries) {
    return (SortedMap<String, String>) super.create(entries);
  }
}
