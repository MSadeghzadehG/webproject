

package com.google.common.collect.testing;

import static com.google.common.collect.testing.features.CollectionFeature.SERIALIZABLE;
import static com.google.common.collect.testing.features.CollectionFeature.SERIALIZABLE_INCLUDING_VIEWS;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.features.Feature;
import com.google.common.testing.SerializableTester;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import junit.framework.TestSuite;


@GwtIncompatible
public class CollectionTestSuiteBuilder<E>
    extends AbstractCollectionTestSuiteBuilder<CollectionTestSuiteBuilder<E>, E> {
  public static <E> CollectionTestSuiteBuilder<E> using(TestCollectionGenerator<E> generator) {
    return new CollectionTestSuiteBuilder<E>().usingGenerator(generator);
  }

  @Override
  protected List<TestSuite> createDerivedSuites(
      FeatureSpecificTestSuiteBuilder<?, ? extends OneSizeTestContainerGenerator<Collection<E>, E>>
          parentBuilder) {
    List<TestSuite> derivedSuites = new ArrayList<>(super.createDerivedSuites(parentBuilder));

    if (parentBuilder.getFeatures().contains(SERIALIZABLE)) {
      derivedSuites.add(
          CollectionTestSuiteBuilder.using(
                  new ReserializedCollectionGenerator<E>(parentBuilder.getSubjectGenerator()))
              .named(getName() + " reserialized")
              .withFeatures(computeReserializedCollectionFeatures(parentBuilder.getFeatures()))
              .suppressing(parentBuilder.getSuppressedTests())
              .createTestSuite());
    }
    return derivedSuites;
  }

  static class ReserializedCollectionGenerator<E> implements TestCollectionGenerator<E> {
    final OneSizeTestContainerGenerator<Collection<E>, E> gen;

    private ReserializedCollectionGenerator(OneSizeTestContainerGenerator<Collection<E>, E> gen) {
      this.gen = gen;
    }

    @Override
    public SampleElements<E> samples() {
      return gen.samples();
    }

    @Override
    public Collection<E> create(Object... elements) {
      return SerializableTester.reserialize(gen.create(elements));
    }

    @Override
    public E[] createArray(int length) {
      return gen.createArray(length);
    }

    @Override
    public Iterable<E> order(List<E> insertionOrder) {
      return gen.order(insertionOrder);
    }
  }

  private static Set<Feature<?>> computeReserializedCollectionFeatures(Set<Feature<?>> features) {
    Set<Feature<?>> derivedFeatures = new HashSet<>();
    derivedFeatures.addAll(features);
    derivedFeatures.remove(SERIALIZABLE);
    derivedFeatures.remove(SERIALIZABLE_INCLUDING_VIEWS);
    return derivedFeatures;
  }
}
