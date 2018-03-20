

package com.google.common.collect.testing.testers;

import static com.google.common.collect.testing.features.CollectionFeature.FAILS_FAST_ON_CONCURRENT_MODIFICATION;
import static com.google.common.collect.testing.features.CollectionFeature.SUPPORTS_ITERATOR_REMOVE;
import static com.google.common.collect.testing.features.CollectionFeature.SUPPORTS_REMOVE;
import static com.google.common.collect.testing.features.CollectionSize.SEVERAL;
import static com.google.common.collect.testing.features.CollectionSize.ZERO;

import com.google.common.annotations.GwtCompatible;
import com.google.common.collect.testing.AbstractCollectionTester;
import com.google.common.collect.testing.features.CollectionFeature;
import com.google.common.collect.testing.features.CollectionSize;
import java.util.Collection;
import java.util.ConcurrentModificationException;
import java.util.Iterator;
import java.util.function.Predicate;
import org.junit.Ignore;


@GwtCompatible
@SuppressWarnings("unchecked") @Ignore public class CollectionRemoveIfTester<E> extends AbstractCollectionTester<E> {
  @CollectionFeature.Require(SUPPORTS_ITERATOR_REMOVE)
  public void testRemoveIf_alwaysFalse() {
    assertFalse("removeIf(x -> false) should return false", collection.removeIf(x -> false));
    expectUnchanged();
  }

  @CollectionFeature.Require(SUPPORTS_ITERATOR_REMOVE)
  @CollectionSize.Require(absent = ZERO)
  public void testRemoveIf_sometimesTrue() {
    assertTrue(
        "removeIf(isEqual(present)) should return true",
        collection.removeIf(Predicate.isEqual(samples.e0())));
    expectMissing(samples.e0());
  }

  @CollectionFeature.Require(SUPPORTS_ITERATOR_REMOVE)
  @CollectionSize.Require(absent = ZERO)
  public void testRemoveIf_allPresent() {
    assertTrue("removeIf(x -> true) should return true", collection.removeIf(x -> true));
    expectContents();
  }

  @CollectionFeature.Require({SUPPORTS_ITERATOR_REMOVE, FAILS_FAST_ON_CONCURRENT_MODIFICATION})
  @CollectionSize.Require(SEVERAL)
  public void testRemoveIfSomeMatchesConcurrentWithIteration() {
    try {
      Iterator<E> iterator = collection.iterator();
      assertTrue(collection.removeIf(Predicate.isEqual(samples.e0())));
      iterator.next();
      fail("Expected ConcurrentModificationException");
    } catch (ConcurrentModificationException expected) {
          }
  }

  @CollectionFeature.Require(absent = SUPPORTS_REMOVE)
  @CollectionSize.Require(ZERO)
  public void testRemoveIf_unsupportedEmptyCollection() {
    try {
      assertFalse(
          "removeIf(Predicate) should return false or throw " + "UnsupportedOperationException",
          collection.removeIf(
              x -> {
                throw new AssertionError("predicate should never be called");
              }));
    } catch (UnsupportedOperationException tolerated) {
    }
    expectUnchanged();
  }

  @CollectionFeature.Require(absent = SUPPORTS_REMOVE)
  @CollectionSize.Require(absent = ZERO)
  public void testRemoveIf_alwaysTrueUnsupported() {
    try {
      collection.removeIf(x -> true);
      fail("removeIf(x -> true) should throw " + "UnsupportedOperationException");
    } catch (UnsupportedOperationException expected) {
    }
    expectUnchanged();
    assertTrue(collection.contains(samples.e0()));
  }
}
