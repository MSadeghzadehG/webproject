

package com.google.common.collect.testing;

import static com.google.common.collect.testing.testers.CollectionAddAllTester.getAddAllNullUnsupportedMethod;
import static com.google.common.collect.testing.testers.CollectionAddTester.getAddNullSupportedMethod;
import static com.google.common.collect.testing.testers.CollectionAddTester.getAddNullUnsupportedMethod;
import static com.google.common.collect.testing.testers.CollectionCreationTester.getCreateWithNullUnsupportedMethod;
import static com.google.common.collect.testing.testers.SetAddTester.getAddSupportedNullPresentMethod;

import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Collection;
import java.util.Set;
import junit.framework.Test;


public class OpenJdk6SetTests extends TestsForSetsInJavaUtil {
  public static Test suite() {
    return new OpenJdk6SetTests().allTests();
  }

  @Override
  protected Collection<Method> suppressForTreeSetNatural() {
    return Arrays.asList(
        getAddNullUnsupportedMethod(),
        getAddAllNullUnsupportedMethod(),
        getCreateWithNullUnsupportedMethod());
  }

  @Override
  protected Collection<Method> suppressForCheckedSet() {
    return Arrays.asList(getAddNullSupportedMethod(), getAddSupportedNullPresentMethod());
  }
}
