

package com.google.common.base.internal;

import java.lang.ref.PhantomReference;
import java.lang.ref.Reference;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


public class Finalizer implements Runnable {

  private static final Logger logger = Logger.getLogger(Finalizer.class.getName());

  
  private static final String FINALIZABLE_REFERENCE = "com.google.common.base.FinalizableReference";

  
  public static void startFinalizer(
      Class<?> finalizableReferenceClass,
      ReferenceQueue<Object> queue,
      PhantomReference<Object> frqReference) {
    
    if (!finalizableReferenceClass.getName().equals(FINALIZABLE_REFERENCE)) {
      throw new IllegalArgumentException("Expected " + FINALIZABLE_REFERENCE + ".");
    }

    Finalizer finalizer = new Finalizer(finalizableReferenceClass, queue, frqReference);
    String threadName = Finalizer.class.getName();
    Thread thread = null;
    if (bigThreadConstructor != null) {
      try {
        boolean inheritThreadLocals = false;
        long defaultStackSize = 0;
        thread =
            bigThreadConstructor.newInstance(
                (ThreadGroup) null, finalizer, threadName, defaultStackSize, inheritThreadLocals);
      } catch (Throwable t) {
        logger.log(
            Level.INFO, "Failed to create a thread without inherited thread-local values", t);
      }
    }
    if (thread == null) {
      thread = new Thread((ThreadGroup) null, finalizer, threadName);
    }
    thread.setDaemon(true);

    try {
      if (inheritableThreadLocals != null) {
        inheritableThreadLocals.set(thread, null);
      }
    } catch (Throwable t) {
      logger.log(
          Level.INFO,
          "Failed to clear thread local values inherited by reference finalizer thread.",
          t);
    }

    thread.start();
  }

  private final WeakReference<Class<?>> finalizableReferenceClassReference;
  private final PhantomReference<Object> frqReference;
  private final ReferenceQueue<Object> queue;

        @NullableDecl
  private static final Constructor<Thread> bigThreadConstructor = getBigThreadConstructor();

  @NullableDecl
  private static final Field inheritableThreadLocals =
      (bigThreadConstructor == null) ? getInheritableThreadLocalsField() : null;

  
  private Finalizer(
      Class<?> finalizableReferenceClass,
      ReferenceQueue<Object> queue,
      PhantomReference<Object> frqReference) {
    this.queue = queue;

    this.finalizableReferenceClassReference =
        new WeakReference<Class<?>>(finalizableReferenceClass);

        this.frqReference = frqReference;
  }

  
  @SuppressWarnings("InfiniteLoopStatement")
  @Override
  public void run() {
    while (true) {
      try {
        if (!cleanUp(queue.remove())) {
          break;
        }
      } catch (InterruptedException e) {
              }
    }
  }

  
  private boolean cleanUp(Reference<?> reference) {
    Method finalizeReferentMethod = getFinalizeReferentMethod();
    if (finalizeReferentMethod == null) {
      return false;
    }
    do {
      
      reference.clear();

      if (reference == frqReference) {
        
        return false;
      }

      try {
        finalizeReferentMethod.invoke(reference);
      } catch (Throwable t) {
        logger.log(Level.SEVERE, "Error cleaning up after reference.", t);
      }

      
    } while ((reference = queue.poll()) != null);
    return true;
  }

  
  @NullableDecl
  private Method getFinalizeReferentMethod() {
    Class<?> finalizableReferenceClass = finalizableReferenceClassReference.get();
    if (finalizableReferenceClass == null) {
      
      return null;
    }
    try {
      return finalizableReferenceClass.getMethod("finalizeReferent");
    } catch (NoSuchMethodException e) {
      throw new AssertionError(e);
    }
  }

  @NullableDecl
  private static Field getInheritableThreadLocalsField() {
    try {
      Field inheritableThreadLocals = Thread.class.getDeclaredField("inheritableThreadLocals");
      inheritableThreadLocals.setAccessible(true);
      return inheritableThreadLocals;
    } catch (Throwable t) {
      logger.log(
          Level.INFO,
          "Couldn't access Thread.inheritableThreadLocals. Reference finalizer threads will "
              + "inherit thread local values.");
      return null;
    }
  }

  @NullableDecl
  private static Constructor<Thread> getBigThreadConstructor() {
    try {
      return Thread.class.getConstructor(
          ThreadGroup.class, Runnable.class, String.class, long.class, boolean.class);
    } catch (Throwable t) {
            return null;
    }
  }
}
