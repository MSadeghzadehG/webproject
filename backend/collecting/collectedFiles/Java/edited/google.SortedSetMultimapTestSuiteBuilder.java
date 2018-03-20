

package com.google.common.collect.testing.google;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.SetMultimap;
import com.google.common.collect.testing.AbstractTester;
import com.google.common.collect.testing.FeatureSpecificTestSuiteBuilder;
import com.google.common.collect.testing.Helpers;
import com.google.common.collect.testing.OneSizeTestContainerGenerator;
import com.google.common.collect.testing.SortedSetTestSuiteBuilder;
import com.google.common.collect.testing.features.CollectionSize;
import com.google.common.collect.testing.features.Feature;
import java.util.Collections;
import java.util.EnumSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Set;
import junit.framework.TestSuite;


@GwtIncompatible
public class SortedSetMultimapTestSuiteBuilder<K, V>
    extends MultimapTestSuiteBuilder<K, V, SetMultimap<K, V>> {

  public static <K, V> SortedSetMultimapTestSuiteBuilder<K, V> using(
      TestSetMultimapGenerator<K, V> generator) {
    SortedSetMultimapTestSuiteBuilder<K, V> result = new SortedSetMultimapTestSuiteBuilder<>();
    result.usingGenerator(generator);
    return result;
  }

  @Override
  protected List<Class<? extends AbstractTester>> getTesters() {
    List<Class<? extends AbstractTester>> testers = Helpers.copyToList(super.getTesters());
    testers.add(SetMultimapAsMapTester.class);
    testers.add(SetMultimapEqualsTester.class);
    testers.add(SetMultimapPutTester.class);
    testers.add(SetMultimapPutAllTester.class);
    testers.add(SetMultimapReplaceValuesTester.class);
    testers.add(SortedSetMultimapAsMapTester.class);
    testers.add(SortedSetMultimapGetTester.class);
    return testers;
  }

  @Override
  TestSuite computeMultimapGetTestSuite(
      FeatureSpecificTestSuiteBuilder<
              ?, ? extends OneSizeTestContainerGenerator<SetMultimap<K, V>, Entry<K, V>>>
          parentBuilder) {
    return SortedSetTestSuiteBuilder.using(
            new SetMultimapTestSuiteBuilder.MultimapGetGenerator<K, V>(
                parentBuilder.getSubjectGenerator()))
        .withFeatures(computeMultimapGetFeatures(parentBuilder.getFeatures()))
        .named(parentBuilder.getName() + ".get[key]")
        .suppressing(parentBuilder.getSuppressedTests())
        .createTestSuite();
  }

  @Override
  TestSuite computeMultimapAsMapGetTestSuite(
      FeatureSpecificTestSuiteBuilder<
              ?, ? extends OneSizeTestContainerGenerator<SetMultimap<K, V>, Entry<K, V>>>
          parentBuilder) {
    Set<Feature<?>> features = computeMultimapAsMapGetFeatures(parentBuilder.getFeatures());
    if (Collections.disjoint(features, EnumSet.allOf(CollectionSize.class))) {
      return new TestSuite();
    } else {
      return SortedSetTestSuiteBuilder.using(
              new SetMultimapTestSuiteBuilder.MultimapAsMapGetGenerator<K, V>(
                  parentBuilder.getSubjectGenerator()))
          .withFeatures(features)
          .named(parentBuilder.getName() + ".asMap[].get[key]")
          .suppressing(parentBuilder.getSuppressedTests())
          .createTestSuite();
    }
  }
}
