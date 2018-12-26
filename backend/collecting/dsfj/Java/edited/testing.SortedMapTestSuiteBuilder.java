

package com.google.common.collect.testing;

import static com.google.common.collect.testing.features.CollectionFeature.KNOWN_ORDER;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.DerivedCollectionGenerators.Bound;
import com.google.common.collect.testing.DerivedCollectionGenerators.SortedMapSubmapTestMapGenerator;
import com.google.common.collect.testing.features.Feature;
import com.google.common.collect.testing.testers.SortedMapNavigationTester;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import junit.framework.TestSuite;


@GwtIncompatible
public class SortedMapTestSuiteBuilder<K, V> extends MapTestSuiteBuilder<K, V> {
  public static <K, V> SortedMapTestSuiteBuilder<K, V> using(
      TestSortedMapGenerator<K, V> generator) {
    SortedMapTestSuiteBuilder<K, V> result = new SortedMapTestSuiteBuilder<>();
    result.usingGenerator(generator);
    return result;
  }

  @Override
  protected List<Class<? extends AbstractTester>> getTesters() {
    List<Class<? extends AbstractTester>> testers = Helpers.copyToList(super.getTesters());
    testers.add(SortedMapNavigationTester.class);
    return testers;
  }

  @Override
  public TestSuite createTestSuite() {
    if (!getFeatures().contains(KNOWN_ORDER)) {
      List<Feature<?>> features = Helpers.copyToList(getFeatures());
      features.add(KNOWN_ORDER);
      withFeatures(features);
    }
    return super.createTestSuite();
  }

  @Override
  protected List<TestSuite> createDerivedSuites(
      FeatureSpecificTestSuiteBuilder<
              ?, ? extends OneSizeTestContainerGenerator<Map<K, V>, Entry<K, V>>>
          parentBuilder) {
    List<TestSuite> derivedSuites = super.createDerivedSuites(parentBuilder);

    if (!parentBuilder.getFeatures().contains(NoRecurse.SUBMAP)) {
      derivedSuites.add(createSubmapSuite(parentBuilder, Bound.NO_BOUND, Bound.EXCLUSIVE));
      derivedSuites.add(createSubmapSuite(parentBuilder, Bound.INCLUSIVE, Bound.NO_BOUND));
      derivedSuites.add(createSubmapSuite(parentBuilder, Bound.INCLUSIVE, Bound.EXCLUSIVE));
    }

    return derivedSuites;
  }

  @Override
  protected SetTestSuiteBuilder<K> createDerivedKeySetSuite(TestSetGenerator<K> keySetGenerator) {
    return keySetGenerator instanceof TestSortedSetGenerator
        ? SortedSetTestSuiteBuilder.using((TestSortedSetGenerator<K>) keySetGenerator)
        : SetTestSuiteBuilder.using(keySetGenerator);
  }

  
  enum NoRecurse implements Feature<Void> {
    SUBMAP,
    DESCENDING;

    @Override
    public Set<Feature<? super Void>> getImpliedFeatures() {
      return Collections.emptySet();
    }
  }

  
  final TestSuite createSubmapSuite(
      final FeatureSpecificTestSuiteBuilder<
              ?, ? extends OneSizeTestContainerGenerator<Map<K, V>, Entry<K, V>>>
          parentBuilder,
      final Bound from,
      final Bound to) {
    final TestSortedMapGenerator<K, V> delegate =
        (TestSortedMapGenerator<K, V>) parentBuilder.getSubjectGenerator().getInnerGenerator();

    List<Feature<?>> features = new ArrayList<>();
    features.add(NoRecurse.SUBMAP);
    features.addAll(parentBuilder.getFeatures());

    return newBuilderUsing(delegate, to, from)
        .named(parentBuilder.getName() + " subMap " + from + "-" + to)
        .withFeatures(features)
        .suppressing(parentBuilder.getSuppressedTests())
        .createTestSuite();
  }

  
  SortedMapTestSuiteBuilder<K, V> newBuilderUsing(
      TestSortedMapGenerator<K, V> delegate, Bound to, Bound from) {
    return using(new SortedMapSubmapTestMapGenerator<K, V>(delegate, to, from));
  }
}
