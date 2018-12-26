

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.SampleElements.Strings;
import java.util.Collection;
import java.util.List;


@GwtCompatible
public abstract class TestStringCollectionGenerator implements TestCollectionGenerator<String> {
  @Override
  public SampleElements<String> samples() {
    return new Strings();
  }

  @Override
  public Collection<String> create(Object... elements) {
    String[] array = new String[elements.length];
    int i = 0;
    for (Object e : elements) {
      array[i++] = (String) e;
    }
    return create(array);
  }

  protected abstract Collection<String> create(String[] elements);

  @Override
  public String[] createArray(int length) {
    return new String[length];
  }

  
  @Override
  public List<String> order(List<String> insertionOrder) {
    return insertionOrder;
  }
}
