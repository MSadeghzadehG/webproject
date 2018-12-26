

package com.google.common.collect;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.SetTestSuiteBuilder;
import com.google.common.collect.testing.TestStringSetGenerator;
import com.google.common.collect.testing.features.CollectionFeature;
import com.google.common.collect.testing.features.CollectionSize;
import com.google.common.collect.testing.features.Feature;
import java.util.Arrays;
import java.util.List;
import java.util.Set;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;


@GwtIncompatible public class CompactLinkedHashSetTest extends TestCase {
  public static Test suite() {
    List<Feature<?>> allFeatures = Arrays.<Feature<?>>asList(
        CollectionSize.ANY,
        CollectionFeature.ALLOWS_NULL_VALUES,
        CollectionFeature.FAILS_FAST_ON_CONCURRENT_MODIFICATION,
        CollectionFeature.GENERAL_PURPOSE,
        CollectionFeature.REMOVE_OPERATIONS,
        CollectionFeature.SERIALIZABLE,
        CollectionFeature.KNOWN_ORDER,
        CollectionFeature.SUPPORTS_ADD,
        CollectionFeature.SUPPORTS_REMOVE);

    TestSuite suite = new TestSuite();
    suite.addTestSuite(CompactLinkedHashSetTest.class);
    suite.addTest(SetTestSuiteBuilder.using(new TestStringSetGenerator() {
      @Override protected Set<String> create(String[] elements) {
        return CompactLinkedHashSet.create(Arrays.asList(elements));
      }
    }).named("CompactLinkedHashSet")
      .withFeatures(allFeatures)
      .createTestSuite());
    return suite;
  }

  public void testDummyMethod() {
      }
}
