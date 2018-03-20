

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.SampleElements.Enums;
import java.util.Collections;
import java.util.List;
import java.util.Set;


@GwtCompatible
public abstract class TestEnumSetGenerator implements TestSetGenerator<AnEnum> {
  @Override
  public SampleElements<AnEnum> samples() {
    return new Enums();
  }

  @Override
  public Set<AnEnum> create(Object... elements) {
    AnEnum[] array = new AnEnum[elements.length];
    int i = 0;
    for (Object e : elements) {
      array[i++] = (AnEnum) e;
    }
    return create(array);
  }

  protected abstract Set<AnEnum> create(AnEnum[] elements);

  @Override
  public AnEnum[] createArray(int length) {
    return new AnEnum[length];
  }

  
  @Override
  public List<AnEnum> order(List<AnEnum> insertionOrder) {
    Collections.sort(insertionOrder);
    return insertionOrder;
  }
}
