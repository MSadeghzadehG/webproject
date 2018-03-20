

package com.google.common.collect;

import com.google.common.annotations.GwtIncompatible;
import java.util.Iterator;
import java.util.List;
import java.util.NoSuchElementException;
import junit.framework.TestCase;


@GwtIncompatible public abstract class AbstractRangeSetTest extends TestCase {
  public static void testInvariants(RangeSet<?> rangeSet) {
    testInvariantsInternal(rangeSet);
    testInvariantsInternal(rangeSet.complement());
  }

  private static <C extends Comparable> void testInvariantsInternal(RangeSet<C> rangeSet) {
    assertEquals(rangeSet.asRanges().isEmpty(), rangeSet.isEmpty());
    assertEquals(rangeSet.asDescendingSetOfRanges().isEmpty(), rangeSet.isEmpty());
    assertEquals(!rangeSet.asRanges().iterator().hasNext(), rangeSet.isEmpty());
    assertEquals(!rangeSet.asDescendingSetOfRanges().iterator().hasNext(), rangeSet.isEmpty());

    List<Range<C>> asRanges = ImmutableList.copyOf(rangeSet.asRanges());

        for (int i = 0; i + 1 < asRanges.size(); i++) {
      Range<C> range1 = asRanges.get(i);
      Range<C> range2 = asRanges.get(i + 1);
      assertFalse(range1.isConnected(range2));
    }

        for (Range<C> range : asRanges) {
      assertFalse(range.isEmpty());
    }

        Iterator<Range<C>> itr = rangeSet.asRanges().iterator();
    Range<C> expectedSpan = null;
    if (itr.hasNext()) {
      expectedSpan = itr.next();
      while (itr.hasNext()) {
        expectedSpan = expectedSpan.span(itr.next());
      }
    }

    try {
      Range<C> span = rangeSet.span();
      assertEquals(expectedSpan, span);
    } catch (NoSuchElementException e) {
      assertNull(expectedSpan);
    }

        assertEquals(Lists.reverse(asRanges), ImmutableList.copyOf(rangeSet.asDescendingSetOfRanges()));
  }
}
