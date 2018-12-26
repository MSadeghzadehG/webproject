

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.MapInterfaceTest;
import java.util.Collection;
import java.util.Map;


@GwtCompatible
public class ImmutableMultimapAsMapImplementsMapTest
    extends AbstractMultimapAsMapImplementsMapTest {

  public ImmutableMultimapAsMapImplementsMapTest() {
    super(false, false, false);
  }

  @Override
  protected Map<String, Collection<Integer>> makeEmptyMap() {
    return ImmutableMultimap.<String, Integer>of().asMap();
  }

  @Override
  protected Map<String, Collection<Integer>> makePopulatedMap() {
    Multimap<String, Integer> delegate = HashMultimap.create();
    populate(delegate);
    return ImmutableMultimap.copyOf(delegate).asMap();
  }
}
