

package com.google.common.collect.testing;

import static com.google.common.collect.testing.testers.CollectionCreationTester.getCreateWithNullUnsupportedMethod;

import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Queue;
import junit.framework.Test;


public class OpenJdk6QueueTests extends TestsForQueuesInJavaUtil {
  public static Test suite() {
    return new OpenJdk6QueueTests().allTests();
  }

  private static final List<Method> PQ_SUPPRESS =
      Arrays.asList(getCreateWithNullUnsupportedMethod());

  @Override
  protected Collection<Method> suppressForPriorityBlockingQueue() {
    return PQ_SUPPRESS;
  }

  @Override
  protected Collection<Method> suppressForPriorityQueue() {
    return PQ_SUPPRESS;
  }
}
