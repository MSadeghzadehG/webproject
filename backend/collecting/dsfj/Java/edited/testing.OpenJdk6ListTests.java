

package com.google.common.collect.testing;

import static com.google.common.collect.testing.testers.CollectionToArrayTester.getToArrayIsPlainObjectArrayMethod;
import static com.google.common.collect.testing.testers.ListAddTester.getAddSupportedNullPresentMethod;
import static com.google.common.collect.testing.testers.ListSetTester.getSetNullSupportedMethod;

import com.google.common.collect.testing.testers.CollectionAddTester;
import com.google.common.collect.testing.testers.ListAddAtIndexTester;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import junit.framework.Test;


public class OpenJdk6ListTests extends TestsForListsInJavaUtil {
  public static Test suite() {
    return new OpenJdk6ListTests().allTests();
  }

  @Override
  protected Collection<Method> suppressForArraysAsList() {
    return Arrays.asList(getToArrayIsPlainObjectArrayMethod());
  }

  @Override
  protected Collection<Method> suppressForCheckedList() {
    return Arrays.asList(
        CollectionAddTester.getAddNullSupportedMethod(),
        getAddSupportedNullPresentMethod(),
        ListAddAtIndexTester.getAddNullSupportedMethod(),
        getSetNullSupportedMethod());
  }
}
