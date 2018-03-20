

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.Collections;
import java.util.List;
import java.util.SortedSet;


@GwtCompatible
public abstract class TestStringSortedSetGenerator extends TestStringSetGenerator
    implements TestSortedSetGenerator<String> {

  @Override
  public SortedSet<String> create(Object... elements) {
    return (SortedSet<String>) super.create(elements);
  }

  @Override
  protected abstract SortedSet<String> create(String[] elements);

  
  @Override
  public List<String> order(List<String> insertionOrder) {
    Collections.sort(insertionOrder);
    return insertionOrder;
  }

  @Override
  public String belowSamplesLesser() {
    return "!! a";
  }

  @Override
  public String belowSamplesGreater() {
    return "!! b";
  }

  @Override
  public String aboveSamplesLesser() {
    return "~~ a";
  }

  @Override
  public String aboveSamplesGreater() {
    return "~~ b";
  }
}
