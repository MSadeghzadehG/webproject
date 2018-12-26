

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.SampleElements.Strings;
import java.util.List;
import java.util.Queue;


@GwtCompatible
public abstract class TestStringQueueGenerator implements TestQueueGenerator<String> {
  @Override
  public SampleElements<String> samples() {
    return new Strings();
  }

  @Override
  public Queue<String> create(Object... elements) {
    String[] array = new String[elements.length];
    int i = 0;
    for (Object e : elements) {
      array[i++] = (String) e;
    }
    return create(array);
  }

  protected abstract Queue<String> create(String[] elements);

  @Override
  public String[] createArray(int length) {
    return new String[length];
  }

  
  @Override
  public List<String> order(List<String> insertionOrder) {
    return insertionOrder;
  }
}
