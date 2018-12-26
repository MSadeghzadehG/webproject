

package com.google.common.util.concurrent;

import java.net.URLClassLoader;
import java.util.concurrent.CancellationException;
import java.util.concurrent.Future;
import junit.framework.TestCase;



public class AbstractFutureCancellationCauseTest extends TestCase {

  private ClassLoader oldClassLoader;
  private URLClassLoader classReloader;

  @Override
  protected void setUp() throws Exception {
        SettableFuture<?> unused = SettableFuture.create();
                System.setProperty("guava.concurrent.generate_cancellation_cause", "true");
    final String concurrentPackage = SettableFuture.class.getPackage().getName();
    classReloader =
        new URLClassLoader(ClassPathUtil.getClassPathUrls()) {
          @Override
          public Class<?> loadClass(String name) throws ClassNotFoundException {
            if (name.startsWith(concurrentPackage)) {
              return super.findClass(name);
            }
            return super.loadClass(name);
          }
        };
    oldClassLoader = Thread.currentThread().getContextClassLoader();
    Thread.currentThread().setContextClassLoader(classReloader);
  }

  @Override
  protected void tearDown() throws Exception {
    classReloader.close();
    Thread.currentThread().setContextClassLoader(oldClassLoader);
    System.clearProperty("guava.concurrent.generate_cancellation_cause");
  }

  public void testCancel_notDoneNoInterrupt() throws Exception {
    Future<?> future = newFutureInstance();
    assertTrue(future.cancel(false));
    assertTrue(future.isCancelled());
    assertTrue(future.isDone());
    try {
      future.get();
      fail("Expected CancellationException");
    } catch (CancellationException e) {
      assertNotNull(e.getCause());
    }
  }

  public void testCancel_notDoneInterrupt() throws Exception {
    Future<?> future = newFutureInstance();
    assertTrue(future.cancel(true));
    assertTrue(future.isCancelled());
    assertTrue(future.isDone());
    try {
      future.get();
      fail("Expected CancellationException");
    } catch (CancellationException e) {
      assertNotNull(e.getCause());
    }
  }

  private Future<?> newFutureInstance() throws Exception {
    return (Future<?>)
        classReloader.loadClass(SettableFuture.class.getName()).getMethod("create").invoke(null);
  }
}
