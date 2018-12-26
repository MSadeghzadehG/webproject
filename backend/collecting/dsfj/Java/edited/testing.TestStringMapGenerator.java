

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;


@GwtCompatible
public abstract class TestStringMapGenerator implements TestMapGenerator<String, String> {

  @Override
  public SampleElements<Entry<String, String>> samples() {
    return new SampleElements<>(
        Helpers.mapEntry("one", "January"),
        Helpers.mapEntry("two", "February"),
        Helpers.mapEntry("three", "March"),
        Helpers.mapEntry("four", "April"),
        Helpers.mapEntry("five", "May"));
  }

  @Override
  public Map<String, String> create(Object... entries) {
    @SuppressWarnings("unchecked")
    Entry<String, String>[] array = new Entry[entries.length];
    int i = 0;
    for (Object o : entries) {
      @SuppressWarnings("unchecked")
      Entry<String, String> e = (Entry<String, String>) o;
      array[i++] = e;
    }
    return create(array);
  }

  protected abstract Map<String, String> create(Entry<String, String>[] entries);

  @Override
  @SuppressWarnings("unchecked")
  public final Entry<String, String>[] createArray(int length) {
    return new Entry[length];
  }

  @Override
  public final String[] createKeyArray(int length) {
    return new String[length];
  }

  @Override
  public final String[] createValueArray(int length) {
    return new String[length];
  }

  
  @Override
  public Iterable<Entry<String, String>> order(List<Entry<String, String>> insertionOrder) {
    return insertionOrder;
  }
}
