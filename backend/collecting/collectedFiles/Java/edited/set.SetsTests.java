

package org.elasticsearch.common.util.set;

import org.elasticsearch.common.collect.Tuple;
import org.elasticsearch.test.ESTestCase;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.greaterThan;

public class SetsTests extends ESTestCase {

    public void testDifference() {
        final int endExclusive = randomIntBetween(0, 256);
        final Tuple<Set<Integer>, Set<Integer>> sets = randomSets(endExclusive);
        final Set<Integer> difference = Sets.difference(sets.v1(), sets.v2());
        assertDifference(endExclusive, sets, difference);
    }

    public void testSortedDifference() {
        final int endExclusive = randomIntBetween(0, 256);
        final Tuple<Set<Integer>, Set<Integer>> sets = randomSets(endExclusive);
        final Set<Integer> difference = Sets.sortedDifference(sets.v1(), sets.v2());
        assertDifference(endExclusive, sets, difference);
        final Iterator<Integer> it = difference.iterator();
        if (it.hasNext()) {
            int current = it.next();
            while (it.hasNext()) {
                final int next = it.next();
                assertThat(next, greaterThan(current));
                current = next;
            }
        }
    }

    
    private void assertDifference(
            final int endExclusive, final Tuple<Set<Integer>, Set<Integer>> sets, final Set<Integer> difference) {
        for (int i = 0; i < endExclusive; i++) {
            assertThat(difference.contains(i), equalTo(sets.v1().contains(i) && !sets.v2().contains(i)));
        }
    }

    
    private Tuple<Set<Integer>, Set<Integer>> randomSets(final int endExclusive) {
        final Set<Integer> left = new HashSet<>(randomSubsetOf(IntStream.range(0, endExclusive).boxed().collect(Collectors.toSet())));
        final Set<Integer> right = new HashSet<>(randomSubsetOf(IntStream.range(0, endExclusive).boxed().collect(Collectors.toSet())));
        return Tuple.tuple(left, right);
    }

}
