

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.MapInterfaceTest;
import java.util.Collection;
import java.util.Map;


@GwtCompatible
public class UnmodifiableMultimapAsMapImplementsMapTest
    extends AbstractMultimapAsMapImplementsMapTest {

  public UnmodifiableMultimapAsMapImplementsMapTest() {
    super(false, true, false);
  }

  @Override
  protected Map<String, Collection<Integer>> makeEmptyMap() {
    return Multimaps.unmodifiableMultimap(LinkedHashMultimap.<String, Integer>create()).asMap();
  }

  @Override
  protected Map<String, Collection<Integer>> makePopulatedMap() {
    Multimap<String, Integer> delegate = LinkedHashMultimap.create();
    populate(delegate);
    return Multimaps.unmodifiableMultimap(delegate).asMap();
  }
}
