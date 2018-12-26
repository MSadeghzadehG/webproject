

package com.google.common.collect.testing;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.features.CollectionSize;
import com.google.common.collect.testing.features.Feature;
import com.google.common.collect.testing.features.FeatureUtil;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Set;
import java.util.logging.Logger;
import junit.framework.TestSuite;


@GwtIncompatible
public abstract class PerCollectionSizeTestSuiteBuilder<
        B extends PerCollectionSizeTestSuiteBuilder<B, G, T, E>,
        G extends TestContainerGenerator<T, E>,
        T,
        E>
    extends FeatureSpecificTestSuiteBuilder<B, G> {
  private static final Logger logger =
      Logger.getLogger(PerCollectionSizeTestSuiteBuilder.class.getName());

  
  @Override
  public TestSuite createTestSuite() {
    checkCanCreate();

    String name = getName();
        Set<Feature<?>> features = Helpers.copyToSet(getFeatures());
    List<Class<? extends AbstractTester>> testers = getTesters();

    logger.fine(" Testing: " + name);

        Set<Feature<?>> sizesToTest = Helpers.<Feature<?>>copyToSet(CollectionSize.values());
    sizesToTest.retainAll(features);
    features.removeAll(sizesToTest);

    FeatureUtil.addImpliedFeatures(sizesToTest);
    sizesToTest.retainAll(
        Arrays.asList(CollectionSize.ZERO, CollectionSize.ONE, CollectionSize.SEVERAL));

    logger.fine("   Sizes: " + formatFeatureSet(sizesToTest));

    if (sizesToTest.isEmpty()) {
      throw new IllegalStateException(
          name
              + ": no CollectionSizes specified (check the argument to "
              + "FeatureSpecificTestSuiteBuilder.withFeatures().)");
    }

    TestSuite suite = new TestSuite(name);
    for (Feature<?> collectionSize : sizesToTest) {
      String oneSizeName =
          Platform.format(
              "%s [collection size: %s]", name, collectionSize.toString().toLowerCase());
      OneSizeGenerator<T, E> oneSizeGenerator =
          new OneSizeGenerator<>(getSubjectGenerator(), (CollectionSize) collectionSize);
      Set<Feature<?>> oneSizeFeatures = Helpers.copyToSet(features);
      oneSizeFeatures.add(collectionSize);
      Set<Method> oneSizeSuppressedTests = getSuppressedTests();

      OneSizeTestSuiteBuilder<T, E> oneSizeBuilder =
          new OneSizeTestSuiteBuilder<T, E>(testers)
              .named(oneSizeName)
              .usingGenerator(oneSizeGenerator)
              .withFeatures(oneSizeFeatures)
              .withSetUp(getSetUp())
              .withTearDown(getTearDown())
              .suppressing(oneSizeSuppressedTests);
      TestSuite oneSizeSuite = oneSizeBuilder.createTestSuite();
      suite.addTest(oneSizeSuite);

      for (TestSuite derivedSuite : createDerivedSuites(oneSizeBuilder)) {
        oneSizeSuite.addTest(derivedSuite);
      }
    }
    return suite;
  }

  protected List<TestSuite> createDerivedSuites(
      FeatureSpecificTestSuiteBuilder<?, ? extends OneSizeTestContainerGenerator<T, E>>
          parentBuilder) {
    return new ArrayList<>();
  }

  
  private static final class OneSizeTestSuiteBuilder<T, E>
      extends FeatureSpecificTestSuiteBuilder<
          OneSizeTestSuiteBuilder<T, E>, OneSizeGenerator<T, E>> {
    private final List<Class<? extends AbstractTester>> testers;

    public OneSizeTestSuiteBuilder(List<Class<? extends AbstractTester>> testers) {
      this.testers = testers;
    }

    @Override
    protected List<Class<? extends AbstractTester>> getTesters() {
      return testers;
    }
  }
}
