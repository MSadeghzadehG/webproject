

package com.google.common.util.concurrent;

import com.google.common.collect.ImmutableSet;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.net.URLClassLoader;
import java.util.Set;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import java.util.concurrent.atomic.AtomicReferenceFieldUpdater;
import junit.framework.TestCase;
import junit.framework.TestSuite;



public class AggregateFutureStateFallbackAtomicHelperTest extends TestCase {

  
  private static final ClassLoader NO_ATOMIC_FIELD_UPDATER =
      getClassLoader(
          ImmutableSet.of(
              AtomicIntegerFieldUpdater.class.getName(),
              AtomicReferenceFieldUpdater.class.getName()));

  public static TestSuite suite() {
                TestSuite suite = new TestSuite(AggregateFutureStateFallbackAtomicHelperTest.class.getName());
    for (Method method : FuturesTest.class.getDeclaredMethods()) {
      if (Modifier.isPublic(method.getModifiers()) && method.getName().startsWith("test")) {
        suite.addTest(
            TestSuite.createTest(
                AggregateFutureStateFallbackAtomicHelperTest.class, method.getName()));
      }
    }
    return suite;
  }

  @Override
  public void runTest() throws Exception {
        checkHelperVersion(getClass().getClassLoader(), "SafeAtomicHelper");
    checkHelperVersion(NO_ATOMIC_FIELD_UPDATER, "SynchronizedAtomicHelper");

            ClassLoader oldClassLoader = Thread.currentThread().getContextClassLoader();
    Thread.currentThread().setContextClassLoader(NO_ATOMIC_FIELD_UPDATER);
    try {
      runTestMethod(NO_ATOMIC_FIELD_UPDATER);
          } finally {
      Thread.currentThread().setContextClassLoader(oldClassLoader);
    }
  }

  private void runTestMethod(ClassLoader classLoader) throws Exception {
    Class<?> test = classLoader.loadClass(FuturesTest.class.getName());
    Object testInstance = test.newInstance();
    test.getMethod("setUp").invoke(testInstance);
    test.getMethod(getName()).invoke(testInstance);
    test.getMethod("tearDown").invoke(testInstance);
  }

  private void checkHelperVersion(ClassLoader classLoader, String expectedHelperClassName)
      throws Exception {
        Class<?> abstractFutureClass = classLoader.loadClass(AggregateFutureState.class.getName());
    Field helperField = abstractFutureClass.getDeclaredField("ATOMIC_HELPER");
    helperField.setAccessible(true);
    assertEquals(expectedHelperClassName, helperField.get(null).getClass().getSimpleName());
  }

  private static ClassLoader getClassLoader(final Set<String> blacklist) {
    final String concurrentPackage = SettableFuture.class.getPackage().getName();
    ClassLoader classLoader = AggregateFutureStateFallbackAtomicHelperTest.class.getClassLoader();
        return new URLClassLoader(ClassPathUtil.getClassPathUrls(), classLoader) {
      @Override
      public Class<?> loadClass(String name) throws ClassNotFoundException {
        if (blacklist.contains(name)) {
          throw new ClassNotFoundException("I'm sorry Dave, I'm afraid I can't do that.");
        }
        if (name.startsWith(concurrentPackage)) {
          Class<?> c = findLoadedClass(name);
          if (c == null) {
            return super.findClass(name);
          }
          return c;
        }
        return super.loadClass(name);
      }
    };
  }
}
