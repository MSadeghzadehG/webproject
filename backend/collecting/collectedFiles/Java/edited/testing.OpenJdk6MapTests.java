

package com.google.common.collect.testing;

import static com.google.common.collect.Lists.newArrayList;
import static com.google.common.collect.testing.testers.CollectionAddAllTester.getAddAllUnsupportedNonePresentMethod;
import static com.google.common.collect.testing.testers.CollectionAddAllTester.getAddAllUnsupportedSomePresentMethod;
import static com.google.common.collect.testing.testers.CollectionAddTester.getAddUnsupportedNotPresentMethod;
import static com.google.common.collect.testing.testers.CollectionCreationTester.getCreateWithNullUnsupportedMethod;
import static com.google.common.collect.testing.testers.MapCreationTester.getCreateWithNullKeyUnsupportedMethod;
import static com.google.common.collect.testing.testers.MapEntrySetTester.getContainsEntryWithIncomparableKeyMethod;
import static com.google.common.collect.testing.testers.MapEntrySetTester.getContainsEntryWithIncomparableValueMethod;
import static com.google.common.collect.testing.testers.MapPutAllTester.getPutAllNullKeyUnsupportedMethod;
import static com.google.common.collect.testing.testers.MapPutTester.getPutNullKeyUnsupportedMethod;

import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import junit.framework.Test;



public class OpenJdk6MapTests extends TestsForMapsInJavaUtil {
  public static Test suite() {
    return new OpenJdk6MapTests().allTests();
  }

  @Override
  protected Collection<Method> suppressForTreeMapNatural() {
    return Arrays.asList(
        getPutNullKeyUnsupportedMethod(),
        getPutAllNullKeyUnsupportedMethod(),
        getCreateWithNullKeyUnsupportedMethod(),
        getCreateWithNullUnsupportedMethod(),         getContainsEntryWithIncomparableKeyMethod(),
        getContainsEntryWithIncomparableValueMethod());
  }

  @Override
  protected Collection<Method> suppressForConcurrentHashMap() {
    
    return Arrays.asList(
        getAddUnsupportedNotPresentMethod(),
        getAddAllUnsupportedNonePresentMethod(),
        getAddAllUnsupportedSomePresentMethod());
  }

  @Override
  protected Collection<Method> suppressForConcurrentSkipListMap() {
    List<Method> methods = newArrayList();
    methods.addAll(super.suppressForConcurrentSkipListMap());
    methods.add(getContainsEntryWithIncomparableKeyMethod());
    methods.add(getContainsEntryWithIncomparableValueMethod());
    return methods;
  }
}
