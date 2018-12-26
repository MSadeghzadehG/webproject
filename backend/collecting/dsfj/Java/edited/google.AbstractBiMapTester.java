

package com.google.common.collect.testing.google;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.BiMap;
import com.google.common.collect.testing.AbstractMapTester;
import com.google.common.collect.testing.Helpers;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map.Entry;
import org.junit.Ignore;


@GwtCompatible
@Ignore public abstract class AbstractBiMapTester<K, V> extends AbstractMapTester<K, V> {

  @Override
  protected BiMap<K, V> getMap() {
    return (BiMap<K, V>) super.getMap();
  }

  static <K, V> Entry<V, K> reverseEntry(Entry<K, V> entry) {
    return Helpers.mapEntry(entry.getValue(), entry.getKey());
  }

  @Override
  protected void expectContents(Collection<Entry<K, V>> expected) {
    super.expectContents(expected);
    List<Entry<V, K>> reversedEntries = new ArrayList<>();
    for (Entry<K, V> entry : expected) {
      reversedEntries.add(reverseEntry(entry));
    }
    Helpers.assertEqualIgnoringOrder(getMap().inverse().entrySet(), reversedEntries);

    for (Entry<K, V> entry : expected) {
      assertEquals(
          "Wrong key for value " + entry.getValue(),
          entry.getKey(),
          getMap().inverse().get(entry.getValue()));
    }
  }

  @Override
  protected void expectMissing(Entry<K, V>... entries) {
    super.expectMissing(entries);
    for (Entry<K, V> entry : entries) {
      Entry<V, K> reversed = reverseEntry(entry);
      BiMap<V, K> inv = getMap().inverse();
      assertFalse(
          "Inverse should not contain entry " + reversed, inv.entrySet().contains(reversed));
      assertFalse(
          "Inverse should not contain key " + reversed.getKey(),
          inv.containsKey(reversed.getKey()));
      assertFalse(
          "Inverse should not contain value " + reversed.getValue(),
          inv.containsValue(reversed.getValue()));
      
      assertNull(
          "Inverse should not return a mapping for key " + reversed.getKey(),
          inv.get(reversed.getKey()));
    }
  }
}
