

package com.google.common.collect;

import static com.google.common.truth.Truth.assertThat;

import com.google.common.annotations.GwtCompatible;
import com.google.common.base.Ascii;
import com.google.common.collect.testing.SpliteratorTester;
import java.util.Arrays;
import java.util.List;
import java.util.Spliterator;
import junit.framework.TestCase;


@GwtCompatible
public class CollectSpliteratorsTest extends TestCase {
  public void testMap() {
    SpliteratorTester.of(
            () ->
                CollectSpliterators.map(
                    Arrays.spliterator(new String[] {"a", "b", "c", "d", "e"}), Ascii::toUpperCase))
        .expect("A", "B", "C", "D", "E");
  }

  public void testFlatMap() {
    SpliteratorTester.of(
            () ->
                CollectSpliterators.flatMap(
                    Arrays.spliterator(new String[] {"abc", "", "de", "f", "g", ""}),
                    (String str) -> Lists.charactersOf(str).spliterator(),
                    Spliterator.SIZED | Spliterator.DISTINCT | Spliterator.NONNULL,
                    7))
        .expect('a', 'b', 'c', 'd', 'e', 'f', 'g');
  }

  public void testMultisetsSpliterator() {
    Multiset<String> multiset = TreeMultiset.create();
    multiset.add("a", 3);
    multiset.add("b", 1);
    multiset.add("c", 2);

    List<String> actualValues = Lists.newArrayList();
    multiset.spliterator().forEachRemaining(actualValues::add);
    assertThat(multiset).containsExactly("a", "a", "a", "b", "c", "c").inOrder();
  }
}
