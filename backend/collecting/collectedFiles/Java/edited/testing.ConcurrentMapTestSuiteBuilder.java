

package com.google.common.collect.testing;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.testers.ConcurrentMapPutIfAbsentTester;
import com.google.common.collect.testing.testers.ConcurrentMapRemoveTester;
import com.google.common.collect.testing.testers.ConcurrentMapReplaceEntryTester;
import com.google.common.collect.testing.testers.ConcurrentMapReplaceTester;
import java.util.Arrays;
import java.util.List;


@GwtIncompatible
public class ConcurrentMapTestSuiteBuilder<K, V> extends MapTestSuiteBuilder<K, V> {
  public static <K, V> ConcurrentMapTestSuiteBuilder<K, V> using(TestMapGenerator<K, V> generator) {
    ConcurrentMapTestSuiteBuilder<K, V> result = new ConcurrentMapTestSuiteBuilder<>();
    result.usingGenerator(generator);
    return result;
  }

  static final List<? extends Class<? extends AbstractTester>> TESTERS =
      Arrays.asList(
          ConcurrentMapPutIfAbsentTester.class,
          ConcurrentMapRemoveTester.class,
          ConcurrentMapReplaceTester.class,
          ConcurrentMapReplaceEntryTester.class);

  @Override
  protected List<Class<? extends AbstractTester>> getTesters() {
    List<Class<? extends AbstractTester>> testers = Helpers.copyToList(super.getTesters());
    testers.addAll(TESTERS);
    return testers;
  }
}
