

package com.google.common.collect.testing;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.testers.CollectionAddAllTester;
import com.google.common.collect.testing.testers.CollectionAddTester;
import com.google.common.collect.testing.testers.CollectionClearTester;
import com.google.common.collect.testing.testers.CollectionContainsAllTester;
import com.google.common.collect.testing.testers.CollectionContainsTester;
import com.google.common.collect.testing.testers.CollectionCreationTester;
import com.google.common.collect.testing.testers.CollectionEqualsTester;
import com.google.common.collect.testing.testers.CollectionIsEmptyTester;
import com.google.common.collect.testing.testers.CollectionIteratorTester;
import com.google.common.collect.testing.testers.CollectionRemoveAllTester;
import com.google.common.collect.testing.testers.CollectionRemoveTester;
import com.google.common.collect.testing.testers.CollectionRetainAllTester;
import com.google.common.collect.testing.testers.CollectionSerializationTester;
import com.google.common.collect.testing.testers.CollectionSizeTester;
import com.google.common.collect.testing.testers.CollectionToArrayTester;
import com.google.common.collect.testing.testers.CollectionToStringTester;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;


@GwtIncompatible
public abstract class AbstractCollectionTestSuiteBuilder<
        B extends AbstractCollectionTestSuiteBuilder<B, E>, E>
    extends PerCollectionSizeTestSuiteBuilder<B, TestCollectionGenerator<E>, Collection<E>, E> {
    @SuppressWarnings("unchecked")
  @Override
  protected List<Class<? extends AbstractTester>> getTesters() {
    return Arrays.<Class<? extends AbstractTester>>asList(
        CollectionAddAllTester.class,
        CollectionAddTester.class,
        CollectionClearTester.class,
        CollectionContainsAllTester.class,
        CollectionContainsTester.class,
        CollectionCreationTester.class,
        CollectionEqualsTester.class,
        CollectionIsEmptyTester.class,
        CollectionIteratorTester.class,
        CollectionRemoveAllTester.class,
        CollectionRemoveTester.class,
        CollectionRetainAllTester.class,
        CollectionSerializationTester.class,
        CollectionSizeTester.class,
        CollectionToArrayTester.class,
        CollectionToStringTester.class);
  }
}
