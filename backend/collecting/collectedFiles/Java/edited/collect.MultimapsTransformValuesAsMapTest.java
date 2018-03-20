

package com.google.common.collect;

import com.google.common.annotations.GwtCompatible;
import com.google.common.base.Functions;
import java.util.Collection;
import java.util.Map;


@GwtCompatible
public class MultimapsTransformValuesAsMapTest extends AbstractMultimapAsMapImplementsMapTest {

  public MultimapsTransformValuesAsMapTest() {
    super(true, true, true);
  }

  @Override
  protected Map<String, Collection<Integer>> makeEmptyMap() {
    return Multimaps.transformValues(
            ArrayListMultimap.<String, Integer>create(), Functions.<Integer>identity())
        .asMap();
  }

  @Override
  protected Map<String, Collection<Integer>> makePopulatedMap() {
    ListMultimap<String, Integer> delegate = ArrayListMultimap.create();
    populate(delegate);
    return Multimaps.transformValues(delegate, Functions.<Integer>identity()).asMap();
  }
}
