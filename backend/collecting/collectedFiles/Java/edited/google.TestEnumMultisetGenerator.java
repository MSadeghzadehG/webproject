

package com.google.common.collect.testing.google;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.Multiset;
import com.google.common.collect.testing.AnEnum;
import com.google.common.collect.testing.SampleElements;
import com.google.common.collect.testing.SampleElements.Enums;
import java.util.Collections;
import java.util.List;


@GwtCompatible
public abstract class TestEnumMultisetGenerator implements TestMultisetGenerator<AnEnum> {
  @Override
  public SampleElements<AnEnum> samples() {
    return new Enums();
  }

  @Override
  public Multiset<AnEnum> create(Object... elements) {
    AnEnum[] array = new AnEnum[elements.length];
    int i = 0;
    for (Object e : elements) {
      array[i++] = (AnEnum) e;
    }
    return create(array);
  }

  protected abstract Multiset<AnEnum> create(AnEnum[] elements);

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
