

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.MapInterfaceTest;
import java.util.Collection;
import java.util.Map;


@GwtCompatible
public class ImmutableSetMultimapAsMapImplementsMapTest
    extends AbstractMultimapAsMapImplementsMapTest {

  public ImmutableSetMultimapAsMapImplementsMapTest() {
    super(false, false, false);
  }

  @Override
  protected Map<String, Collection<Integer>> makeEmptyMap() {
    return ImmutableSetMultimap.<String, Integer>of().asMap();
  }

  @Override
  protected Map<String, Collection<Integer>> makePopulatedMap() {
    Multimap<String, Integer> delegate = HashMultimap.create();
    populate(delegate);
    return ImmutableSetMultimap.copyOf(delegate).asMap();
  }
}
