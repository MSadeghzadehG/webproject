

package com.google.common.collect.testing;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.features.CollectionFeature;
import com.google.common.collect.testing.features.CollectionSize;
import java.lang.reflect.Method;
import java.util.ArrayDeque;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.PriorityQueue;
import java.util.Queue;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.PriorityBlockingQueue;
import junit.framework.Test;
import junit.framework.TestSuite;


@GwtIncompatible
public class TestsForQueuesInJavaUtil {
  public static Test suite() {
    return new TestsForQueuesInJavaUtil().allTests();
  }

  public Test allTests() {
    TestSuite suite = new TestSuite();
    suite.addTest(testsForArrayDeque());
    suite.addTest(testsForLinkedList());
    suite.addTest(testsForArrayBlockingQueue());
    suite.addTest(testsForConcurrentLinkedQueue());
    suite.addTest(testsForLinkedBlockingDeque());
    suite.addTest(testsForLinkedBlockingQueue());
    suite.addTest(testsForPriorityBlockingQueue());
    suite.addTest(testsForPriorityQueue());
    return suite;
  }

  protected Collection<Method> suppressForArrayDeque() {
    return Collections.emptySet();
  }

  protected Collection<Method> suppressForLinkedList() {
    return Collections.emptySet();
  }

  protected Collection<Method> suppressForArrayBlockingQueue() {
    return Collections.emptySet();
  }

  protected Collection<Method> suppressForConcurrentLinkedQueue() {
    return Collections.emptySet();
  }

  protected Collection<Method> suppressForLinkedBlockingDeque() {
    return Collections.emptySet();
  }

  protected Collection<Method> suppressForLinkedBlockingQueue() {
    return Collections.emptySet();
  }

  protected Collection<Method> suppressForPriorityBlockingQueue() {
    return Collections.emptySet();
  }

  protected Collection<Method> suppressForPriorityQueue() {
    return Collections.emptySet();
  }

  public Test testsForArrayDeque() {
    return QueueTestSuiteBuilder.using(
            new TestStringQueueGenerator() {
              @Override
              public Queue<String> create(String[] elements) {
                return new ArrayDeque<>(MinimalCollection.of(elements));
              }
            })
        .named("ArrayDeque")
        .withFeatures(
            CollectionFeature.GENERAL_PURPOSE, CollectionFeature.KNOWN_ORDER, CollectionSize.ANY)
        .suppressing(suppressForArrayDeque())
        .createTestSuite();
  }

  public Test testsForLinkedList() {
    return QueueTestSuiteBuilder.using(
            new TestStringQueueGenerator() {
              @Override
              public Queue<String> create(String[] elements) {
                return new LinkedList<>(MinimalCollection.of(elements));
              }
            })
        .named("LinkedList")
        .withFeatures(
            CollectionFeature.GENERAL_PURPOSE,
            CollectionFeature.ALLOWS_NULL_VALUES,
            CollectionFeature.KNOWN_ORDER,
            CollectionSize.ANY)
        .skipCollectionTests()         .suppressing(suppressForLinkedList())
        .createTestSuite();
  }

  public Test testsForArrayBlockingQueue() {
    return QueueTestSuiteBuilder.using(
            new TestStringQueueGenerator() {
              @Override
              public Queue<String> create(String[] elements) {
                return new ArrayBlockingQueue<>(100, false, MinimalCollection.of(elements));
              }
            })
        .named("ArrayBlockingQueue")
        .withFeatures(
            CollectionFeature.GENERAL_PURPOSE, CollectionFeature.KNOWN_ORDER, CollectionSize.ANY)
        .suppressing(suppressForArrayBlockingQueue())
        .createTestSuite();
  }

  public Test testsForConcurrentLinkedQueue() {
    return QueueTestSuiteBuilder.using(
            new TestStringQueueGenerator() {
              @Override
              public Queue<String> create(String[] elements) {
                return new ConcurrentLinkedQueue<>(MinimalCollection.of(elements));
              }
            })
        .named("ConcurrentLinkedQueue")
        .withFeatures(
            CollectionFeature.GENERAL_PURPOSE, CollectionFeature.KNOWN_ORDER, CollectionSize.ANY)
        .suppressing(suppressForConcurrentLinkedQueue())
        .createTestSuite();
  }

  public Test testsForLinkedBlockingDeque() {
    return QueueTestSuiteBuilder.using(
            new TestStringQueueGenerator() {
              @Override
              public Queue<String> create(String[] elements) {
                return new LinkedBlockingDeque<>(MinimalCollection.of(elements));
              }
            })
        .named("LinkedBlockingDeque")
        .withFeatures(
            CollectionFeature.GENERAL_PURPOSE, CollectionFeature.KNOWN_ORDER, CollectionSize.ANY)
        .suppressing(suppressForLinkedBlockingDeque())
        .createTestSuite();
  }

  public Test testsForLinkedBlockingQueue() {
    return QueueTestSuiteBuilder.using(
            new TestStringQueueGenerator() {
              @Override
              public Queue<String> create(String[] elements) {
                return new LinkedBlockingQueue<>(MinimalCollection.of(elements));
              }
            })
        .named("LinkedBlockingQueue")
        .withFeatures(
            CollectionFeature.GENERAL_PURPOSE, CollectionFeature.KNOWN_ORDER, CollectionSize.ANY)
        .suppressing(suppressForLinkedBlockingQueue())
        .createTestSuite();
  }

        
  public Test testsForPriorityBlockingQueue() {
    return QueueTestSuiteBuilder.using(
            new TestStringQueueGenerator() {
              @Override
              public Queue<String> create(String[] elements) {
                return new PriorityBlockingQueue<>(MinimalCollection.of(elements));
              }
            })
        .named("PriorityBlockingQueue")
        .withFeatures(CollectionFeature.GENERAL_PURPOSE, CollectionSize.ANY)
        .suppressing(suppressForPriorityBlockingQueue())
        .createTestSuite();
  }

  public Test testsForPriorityQueue() {
    return QueueTestSuiteBuilder.using(
            new TestStringQueueGenerator() {
              @Override
              public Queue<String> create(String[] elements) {
                return new PriorityQueue<>(MinimalCollection.of(elements));
              }
            })
        .named("PriorityQueue")
        .withFeatures(CollectionFeature.GENERAL_PURPOSE, CollectionSize.ANY)
        .suppressing(suppressForPriorityQueue())
        .createTestSuite();
  }
}
