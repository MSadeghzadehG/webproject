

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.SampleElements.Strings;
import java.util.List;
import java.util.Set;


@GwtCompatible
public abstract class TestStringSetGenerator implements TestSetGenerator<String> {
  @Override
  public SampleElements<String> samples() {
    return new Strings();
  }

  @Override
  public Set<String> create(Object... elements) {
    String[] array = new String[elements.length];
    int i = 0;
    for (Object e : elements) {
      array[i++] = (String) e;
    }
    return create(array);
  }

  protected abstract Set<String> create(String[] elements);

  @Override
  public String[] createArray(int length) {
    return new String[length];
  }

  
  @Override
  public List<String> order(List<String> insertionOrder) {
    return insertionOrder;
  }
}
