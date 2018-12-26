



package com.google.common.util.concurrent;

import static java.util.concurrent.TimeUnit.MILLISECONDS;
import static java.util.concurrent.TimeUnit.NANOSECONDS;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.security.CodeSource;
import java.security.Permission;
import java.security.PermissionCollection;
import java.security.Permissions;
import java.security.Policy;
import java.security.ProtectionDomain;
import java.security.SecurityPermission;
import java.util.Arrays;
import java.util.Date;
import java.util.NoSuchElementException;
import java.util.PropertyPermission;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.Callable;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.RejectedExecutionHandler;
import java.util.concurrent.Semaphore;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicReference;
import junit.framework.AssertionFailedError;
import junit.framework.TestCase;


abstract class JSR166TestCase extends TestCase {
  private static final boolean useSecurityManager = Boolean.getBoolean("jsr166.useSecurityManager");

  protected static final boolean expensiveTests = Boolean.getBoolean("jsr166.expensiveTests");

  
  private static final boolean profileTests = Boolean.getBoolean("jsr166.profileTests");

  
  private static final long profileThreshold = Long.getLong("jsr166.profileThreshold", 100);

  protected void runTest() throws Throwable {
    if (profileTests) runTestProfiled();
    else super.runTest();
  }

  protected void runTestProfiled() throws Throwable {
    long t0 = System.nanoTime();
    try {
      super.runTest();
    } finally {
      long elapsedMillis = (System.nanoTime() - t0) / (1000L * 1000L);
      if (elapsedMillis >= profileThreshold)
        System.out.printf("%n%s: %d%n", toString(), elapsedMillis);
    }
  }

                    
                
                        
                                                                                                                                            
  public static long SHORT_DELAY_MS;
  public static long SMALL_DELAY_MS;
  public static long MEDIUM_DELAY_MS;
  public static long LONG_DELAY_MS;

  
  protected long getShortDelay() {
    return 50;
  }

  
  protected void setDelays() {
    SHORT_DELAY_MS = getShortDelay();
    SMALL_DELAY_MS = SHORT_DELAY_MS * 5;
    MEDIUM_DELAY_MS = SHORT_DELAY_MS * 10;
    LONG_DELAY_MS = SHORT_DELAY_MS * 200;
  }

  
  long timeoutMillis() {
    return SHORT_DELAY_MS / 4;
  }

  
  Date delayedDate(long delayMillis) {
    return new Date(System.currentTimeMillis() + delayMillis);
  }

  
  private final AtomicReference<Throwable> threadFailure = new AtomicReference<>(null);

  
  public void threadRecordFailure(Throwable t) {
    threadFailure.compareAndSet(null, t);
  }

  public void setUp() {
    setDelays();
  }

  
  public void tearDown() throws Exception {
    Throwable t = threadFailure.getAndSet(null);
    if (t != null) {
      if (t instanceof Error) throw (Error) t;
      else if (t instanceof RuntimeException) throw (RuntimeException) t;
      else if (t instanceof Exception) throw (Exception) t;
      else {
        AssertionFailedError afe = new AssertionFailedError(t.toString());
        afe.initCause(t);
        throw afe;
      }
    }

    if (Thread.interrupted()) throw new AssertionFailedError("interrupt status set in main thread");
  }

  
  public void threadFail(String reason) {
    try {
      fail(reason);
    } catch (AssertionFailedError t) {
      threadRecordFailure(t);
      fail(reason);
    }
  }

  
  public void threadAssertTrue(boolean b) {
    try {
      assertTrue(b);
    } catch (AssertionFailedError t) {
      threadRecordFailure(t);
      throw t;
    }
  }

  
  public void threadAssertFalse(boolean b) {
    try {
      assertFalse(b);
    } catch (AssertionFailedError t) {
      threadRecordFailure(t);
      throw t;
    }
  }

  
  public void threadAssertNull(Object x) {
    try {
      assertNull(x);
    } catch (AssertionFailedError t) {
      threadRecordFailure(t);
      throw t;
    }
  }

  
  public void threadAssertEquals(long x, long y) {
    try {
      assertEquals(x, y);
    } catch (AssertionFailedError t) {
      threadRecordFailure(t);
      throw t;
    }
  }

  
  public void threadAssertEquals(Object x, Object y) {
    try {
      assertEquals(x, y);
    } catch (AssertionFailedError t) {
      threadRecordFailure(t);
      throw t;
    } catch (Throwable t) {
      threadUnexpectedException(t);
    }
  }

  
  public void threadAssertSame(Object x, Object y) {
    try {
      assertSame(x, y);
    } catch (AssertionFailedError t) {
      threadRecordFailure(t);
      throw t;
    }
  }

  
  public void threadShouldThrow() {
    threadFail("should throw exception");
  }

  
  public void threadShouldThrow(String exceptionName) {
    threadFail("should throw " + exceptionName);
  }

  
  public void threadUnexpectedException(Throwable t) {
    threadRecordFailure(t);
    t.printStackTrace();
    if (t instanceof RuntimeException) throw (RuntimeException) t;
    else if (t instanceof Error) throw (Error) t;
    else {
      AssertionFailedError afe = new AssertionFailedError("unexpected exception: " + t);
      afe.initCause(t);
      throw afe;
    }
  }

  
  static void delay(long millis) throws InterruptedException {
    long startTime = System.nanoTime();
    long ns = millis * 1000 * 1000;
    for (; ; ) {
      if (millis > 0L) Thread.sleep(millis);
      else       Thread.yield();
      long d = ns - (System.nanoTime() - startTime);
      if (d > 0L) millis = d / (1000 * 1000);
      else break;
    }
  }

  
  void joinPool(ExecutorService exec) {
    try {
      exec.shutdown();
      assertTrue(
          "ExecutorService did not terminate in a timely manner",
          exec.awaitTermination(2 * LONG_DELAY_MS, MILLISECONDS));
    } catch (SecurityException ok) {
          } catch (InterruptedException ie) {
      fail("Unexpected InterruptedException");
    }
  }

  
  void assertThreadStaysAlive(Thread thread) {
    assertThreadStaysAlive(thread, timeoutMillis());
  }

  
  void assertThreadStaysAlive(Thread thread, long millis) {
    try {
            delay(millis);
      assertTrue(thread.isAlive());
    } catch (InterruptedException ie) {
      fail("Unexpected InterruptedException");
    }
  }

  
  void assertThreadsStayAlive(Thread... threads) {
    assertThreadsStayAlive(timeoutMillis(), threads);
  }

  
  void assertThreadsStayAlive(long millis, Thread... threads) {
    try {
            delay(millis);
      for (Thread thread : threads) assertTrue(thread.isAlive());
    } catch (InterruptedException ie) {
      fail("Unexpected InterruptedException");
    }
  }

  
  void assertFutureTimesOut(Future future) {
    assertFutureTimesOut(future, timeoutMillis());
  }

  
  void assertFutureTimesOut(Future future, long timeoutMillis) {
    long startTime = System.nanoTime();
    try {
      future.get(timeoutMillis, MILLISECONDS);
      shouldThrow();
    } catch (TimeoutException success) {
    } catch (Exception e) {
      threadUnexpectedException(e);
    } finally {
      future.cancel(true);
    }
    assertTrue(millisElapsedSince(startTime) >= timeoutMillis);
  }

  
  public void shouldThrow() {
    fail("Should throw exception");
  }

  
  public void shouldThrow(String exceptionName) {
    fail("Should throw " + exceptionName);
  }

  
  public static final int SIZE = 20;

  
  public static final Integer zero = new Integer(0);
  public static final Integer one = new Integer(1);
  public static final Integer two = new Integer(2);
  public static final Integer three = new Integer(3);
  public static final Integer four = new Integer(4);
  public static final Integer five = new Integer(5);
  public static final Integer six = new Integer(6);
  public static final Integer seven = new Integer(7);
  public static final Integer eight = new Integer(8);
  public static final Integer nine = new Integer(9);
  public static final Integer m1 = new Integer(-1);
  public static final Integer m2 = new Integer(-2);
  public static final Integer m3 = new Integer(-3);
  public static final Integer m4 = new Integer(-4);
  public static final Integer m5 = new Integer(-5);
  public static final Integer m6 = new Integer(-6);
  public static final Integer m10 = new Integer(-10);

  
  public void runWithPermissions(Runnable r, Permission... permissions) {
    SecurityManager sm = System.getSecurityManager();
    if (sm == null) {
      r.run();
      Policy savedPolicy = Policy.getPolicy();
      try {
        Policy.setPolicy(permissivePolicy());
        System.setSecurityManager(new SecurityManager());
        runWithPermissions(r, permissions);
      } finally {
        System.setSecurityManager(null);
        Policy.setPolicy(savedPolicy);
      }
    } else {
      Policy savedPolicy = Policy.getPolicy();
      AdjustablePolicy policy = new AdjustablePolicy(permissions);
      Policy.setPolicy(policy);

      try {
        r.run();
      } finally {
        policy.addPermission(new SecurityPermission("setPolicy"));
        Policy.setPolicy(savedPolicy);
      }
    }
  }

  
  public void runWithoutPermissions(Runnable r) {
    runWithPermissions(r);
  }

  
  public static class AdjustablePolicy extends java.security.Policy {
    Permissions perms = new Permissions();

    AdjustablePolicy(Permission... permissions) {
      for (Permission permission : permissions) perms.add(permission);
    }

    void addPermission(Permission perm) {
      perms.add(perm);
    }

    void clearPermissions() {
      perms = new Permissions();
    }

    public PermissionCollection getPermissions(CodeSource cs) {
      return perms;
    }

    public PermissionCollection getPermissions(ProtectionDomain pd) {
      return perms;
    }

    public boolean implies(ProtectionDomain pd, Permission p) {
      return perms.implies(p);
    }

    public void refresh() {}
  }

  
  public static Policy permissivePolicy() {
    return new AdjustablePolicy
        (
        new RuntimePermission("modifyThread"),
        new RuntimePermission("getClassLoader"),
        new RuntimePermission("setContextClassLoader"),
                new SecurityPermission("getPolicy"),
        new SecurityPermission("setPolicy"),
        new RuntimePermission("setSecurityManager"),
                new RuntimePermission("accessDeclaredMembers"),
        new PropertyPermission("*", "read"),
        new java.io.FilePermission("<<ALL FILES>>", "read"));
  }

  
  void sleep(long millis) {
    try {
      delay(millis);
    } catch (InterruptedException ie) {
      AssertionFailedError afe = new AssertionFailedError("Unexpected InterruptedException");
      afe.initCause(ie);
      throw afe;
    }
  }

  
  void waitForThreadToEnterWaitState(Thread thread, long timeoutMillis) {
    long startTime = System.nanoTime();
    for (; ; ) {
      Thread.State s = thread.getState();
      if (s == Thread.State.BLOCKED || s == Thread.State.WAITING || s == Thread.State.TIMED_WAITING)
        return;
      else if (s == Thread.State.TERMINATED) fail("Unexpected thread termination");
      else if (millisElapsedSince(startTime) > timeoutMillis) {
        threadAssertTrue(thread.isAlive());
        return;
      }
      Thread.yield();
    }
  }

  
  void waitForThreadToEnterWaitState(Thread thread) {
    waitForThreadToEnterWaitState(thread, LONG_DELAY_MS);
  }

  
  long millisElapsedSince(long startNanoTime) {
    return NANOSECONDS.toMillis(System.nanoTime() - startNanoTime);
  }

  
  Thread newStartedThread(Runnable runnable) {
    Thread t = new Thread(runnable);
    t.setDaemon(true);
    t.start();
    return t;
  }

  
  void awaitTermination(Thread t, long timeoutMillis) {
    try {
      t.join(timeoutMillis);
    } catch (InterruptedException ie) {
      threadUnexpectedException(ie);
    } finally {
      if (t.getState() != Thread.State.TERMINATED) {
        t.interrupt();
        fail("Test timed out");
      }
    }
  }

  
  void awaitTermination(Thread t) {
    awaitTermination(t, LONG_DELAY_MS);
  }

  
  public abstract class CheckedRunnable implements Runnable {
    protected abstract void realRun() throws Throwable;

    public final void run() {
      try {
        realRun();
      } catch (Throwable t) {
        threadUnexpectedException(t);
      }
    }
  }

  public abstract class RunnableShouldThrow implements Runnable {
    protected abstract void realRun() throws Throwable;

    final Class<?> exceptionClass;

    <T extends Throwable> RunnableShouldThrow(Class<T> exceptionClass) {
      this.exceptionClass = exceptionClass;
    }

    public final void run() {
      try {
        realRun();
        threadShouldThrow(exceptionClass.getSimpleName());
      } catch (Throwable t) {
        if (!exceptionClass.isInstance(t)) threadUnexpectedException(t);
      }
    }
  }

  public abstract class ThreadShouldThrow extends Thread {
    protected abstract void realRun() throws Throwable;

    final Class<?> exceptionClass;

    <T extends Throwable> ThreadShouldThrow(Class<T> exceptionClass) {
      this.exceptionClass = exceptionClass;
    }

    public final void run() {
      try {
        realRun();
        threadShouldThrow(exceptionClass.getSimpleName());
      } catch (Throwable t) {
        if (!exceptionClass.isInstance(t)) threadUnexpectedException(t);
      }
    }
  }

  public abstract class CheckedInterruptedRunnable implements Runnable {
    protected abstract void realRun() throws Throwable;

    public final void run() {
      try {
        realRun();
        threadShouldThrow("InterruptedException");
      } catch (InterruptedException success) {
        threadAssertFalse(Thread.interrupted());
      } catch (Throwable t) {
        threadUnexpectedException(t);
      }
    }
  }

  public abstract class CheckedCallable<T> implements Callable<T> {
    protected abstract T realCall() throws Throwable;

    public final T call() {
      try {
        return realCall();
      } catch (Throwable t) {
        threadUnexpectedException(t);
        return null;
      }
    }
  }

  public abstract class CheckedInterruptedCallable<T> implements Callable<T> {
    protected abstract T realCall() throws Throwable;

    public final T call() {
      try {
        T result = realCall();
        threadShouldThrow("InterruptedException");
        return result;
      } catch (InterruptedException success) {
        threadAssertFalse(Thread.interrupted());
      } catch (Throwable t) {
        threadUnexpectedException(t);
      }
      return null;
    }
  }

  public static class NoOpRunnable implements Runnable {
    public void run() {}
  }

  public static class NoOpCallable implements Callable {
    public Object call() {
      return Boolean.TRUE;
    }
  }

  public static final String TEST_STRING = "a test string";

  public static class StringTask implements Callable<String> {
    public String call() {
      return TEST_STRING;
    }
  }

  public Callable<String> latchAwaitingStringTask(final CountDownLatch latch) {
    return new CheckedCallable<String>() {
      protected String realCall() {
        try {
          latch.await();
        } catch (InterruptedException quittingTime) {
        }
        return TEST_STRING;
      }
    };
  }

  public Runnable awaiter(final CountDownLatch latch) {
    return new CheckedRunnable() {
      public void realRun() throws InterruptedException {
        await(latch);
      }
    };
  }

  public void await(CountDownLatch latch) {
    try {
      assertTrue(latch.await(LONG_DELAY_MS, MILLISECONDS));
    } catch (Throwable t) {
      threadUnexpectedException(t);
    }
  }

  public void await(Semaphore semaphore) {
    try {
      assertTrue(semaphore.tryAcquire(LONG_DELAY_MS, MILLISECONDS));
    } catch (Throwable t) {
      threadUnexpectedException(t);
    }
  }

            
                      
  public static class NPETask implements Callable<String> {
    public String call() {
      throw new NullPointerException();
    }
  }

  public static class CallableOne implements Callable<Integer> {
    public Integer call() {
      return one;
    }
  }

  public class ShortRunnable extends CheckedRunnable {
    protected void realRun() throws Throwable {
      delay(SHORT_DELAY_MS);
    }
  }

  public class ShortInterruptedRunnable extends CheckedInterruptedRunnable {
    protected void realRun() throws InterruptedException {
      delay(SHORT_DELAY_MS);
    }
  }

  public class SmallRunnable extends CheckedRunnable {
    protected void realRun() throws Throwable {
      delay(SMALL_DELAY_MS);
    }
  }

  public class SmallPossiblyInterruptedRunnable extends CheckedRunnable {
    protected void realRun() {
      try {
        delay(SMALL_DELAY_MS);
      } catch (InterruptedException ok) {
      }
    }
  }

  public class SmallCallable extends CheckedCallable {
    protected Object realCall() throws InterruptedException {
      delay(SMALL_DELAY_MS);
      return Boolean.TRUE;
    }
  }

  public class MediumRunnable extends CheckedRunnable {
    protected void realRun() throws Throwable {
      delay(MEDIUM_DELAY_MS);
    }
  }

  public class MediumInterruptedRunnable extends CheckedInterruptedRunnable {
    protected void realRun() throws InterruptedException {
      delay(MEDIUM_DELAY_MS);
    }
  }

  public Runnable possiblyInterruptedRunnable(final long timeoutMillis) {
    return new CheckedRunnable() {
      protected void realRun() {
        try {
          delay(timeoutMillis);
        } catch (InterruptedException ok) {
        }
      }
    };
  }

  public class MediumPossiblyInterruptedRunnable extends CheckedRunnable {
    protected void realRun() {
      try {
        delay(MEDIUM_DELAY_MS);
      } catch (InterruptedException ok) {
      }
    }
  }

  public class LongPossiblyInterruptedRunnable extends CheckedRunnable {
    protected void realRun() {
      try {
        delay(LONG_DELAY_MS);
      } catch (InterruptedException ok) {
      }
    }
  }

  
  public static class SimpleThreadFactory implements ThreadFactory {
    public Thread newThread(Runnable r) {
      return new Thread(r);
    }
  }

  public interface TrackedRunnable extends Runnable {
    boolean isDone();
  }

  public static TrackedRunnable trackedRunnable(final long timeoutMillis) {
    return new TrackedRunnable() {
      private volatile boolean done = false;

      public boolean isDone() {
        return done;
      }

      public void run() {
        try {
          delay(timeoutMillis);
          done = true;
        } catch (InterruptedException ok) {
        }
      }
    };
  }

  public static class TrackedShortRunnable implements Runnable {
    public volatile boolean done = false;

    public void run() {
      try {
        delay(SHORT_DELAY_MS);
        done = true;
      } catch (InterruptedException ok) {
      }
    }
  }

  public static class TrackedSmallRunnable implements Runnable {
    public volatile boolean done = false;

    public void run() {
      try {
        delay(SMALL_DELAY_MS);
        done = true;
      } catch (InterruptedException ok) {
      }
    }
  }

  public static class TrackedMediumRunnable implements Runnable {
    public volatile boolean done = false;

    public void run() {
      try {
        delay(MEDIUM_DELAY_MS);
        done = true;
      } catch (InterruptedException ok) {
      }
    }
  }

  public static class TrackedLongRunnable implements Runnable {
    public volatile boolean done = false;

    public void run() {
      try {
        delay(LONG_DELAY_MS);
        done = true;
      } catch (InterruptedException ok) {
      }
    }
  }

  public static class TrackedNoOpRunnable implements Runnable {
    public volatile boolean done = false;

    public void run() {
      done = true;
    }
  }

  public static class TrackedCallable implements Callable {
    public volatile boolean done = false;

    public Object call() {
      try {
        delay(SMALL_DELAY_MS);
        done = true;
      } catch (InterruptedException ok) {
      }
      return Boolean.TRUE;
    }
  }

          
                
          
                  
  
  public static class NoOpREHandler implements RejectedExecutionHandler {
    public void rejectedExecution(Runnable r, ThreadPoolExecutor executor) {}
  }

  
  public class CheckedBarrier extends CyclicBarrier {
    public CheckedBarrier(int parties) {
      super(parties);
    }

    public int await() {
      try {
        return super.await(2 * LONG_DELAY_MS, MILLISECONDS);
      } catch (TimeoutException e) {
        throw new AssertionFailedError("timed out");
      } catch (Exception e) {
        AssertionFailedError afe = new AssertionFailedError("Unexpected exception: " + e);
        afe.initCause(e);
        throw afe;
      }
    }
  }

  void checkEmpty(BlockingQueue q) {
    try {
      assertTrue(q.isEmpty());
      assertEquals(0, q.size());
      assertNull(q.peek());
      assertNull(q.poll());
      assertNull(q.poll(0, MILLISECONDS));
      assertEquals("[]", q.toString());
      assertTrue(Arrays.equals(q.toArray(), new Object[0]));
      assertFalse(q.iterator().hasNext());
      try {
        q.element();
        shouldThrow();
      } catch (NoSuchElementException success) {
      }
      try {
        q.iterator().next();
        shouldThrow();
      } catch (NoSuchElementException success) {
      }
      try {
        q.remove();
        shouldThrow();
      } catch (NoSuchElementException success) {
      }
    } catch (InterruptedException ie) {
      threadUnexpectedException(ie);
    }
  }

  @SuppressWarnings("unchecked")
  <T> T serialClone(T o) {
    try {
      ByteArrayOutputStream bos = new ByteArrayOutputStream();
      ObjectOutputStream oos = new ObjectOutputStream(bos);
      oos.writeObject(o);
      oos.flush();
      oos.close();
      ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(bos.toByteArray()));
      T clone = (T) ois.readObject();
      assertSame(o.getClass(), clone.getClass());
      return clone;
    } catch (Throwable t) {
      threadUnexpectedException(t);
      return null;
    }
  }
}
