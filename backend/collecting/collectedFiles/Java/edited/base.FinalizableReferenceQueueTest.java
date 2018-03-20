

package com.google.common.base;

import com.google.common.base.internal.Finalizer;
import com.google.common.testing.GcFinalization;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.Arrays;
import java.util.Collections;
import junit.framework.TestCase;


public class FinalizableReferenceQueueTest extends TestCase {

  private FinalizableReferenceQueue frq;

  @Override
  protected void tearDown() throws Exception {
    frq = null;
  }

  public void testFinalizeReferentCalled() {
    final MockReference reference = new MockReference(frq = new FinalizableReferenceQueue());

    GcFinalization.awaitDone(
        new GcFinalization.FinalizationPredicate() {
          public boolean isDone() {
            return reference.finalizeReferentCalled;
          }
        });
  }

  static class MockReference extends FinalizableWeakReference<Object> {

    volatile boolean finalizeReferentCalled;

    MockReference(FinalizableReferenceQueue frq) {
      super(new Object(), frq);
    }

    @Override
    public void finalizeReferent() {
      finalizeReferentCalled = true;
    }
  }

  
  private WeakReference<ReferenceQueue<Object>> queueReference;

  public void testThatFinalizerStops() {
    weaklyReferenceQueue();
    GcFinalization.awaitClear(queueReference);
  }

  
  FinalizableWeakReference<Object> reference;

  
  private void weaklyReferenceQueue() {
    frq = new FinalizableReferenceQueue();
    queueReference = new WeakReference<>(frq.queue);

    
    reference =
        new FinalizableWeakReference<Object>(new Object(), frq) {
          @Override
          public void finalizeReferent() {
            reference = null;
            frq = null;
          }
        };
  }

  @AndroidIncompatible   public void testDecoupledLoader() {
    FinalizableReferenceQueue.DecoupledLoader decoupledLoader =
        new FinalizableReferenceQueue.DecoupledLoader() {
          @Override
          URLClassLoader newLoader(URL base) {
            return new DecoupledClassLoader(new URL[] {base});
          }
        };

    Class<?> finalizerCopy = decoupledLoader.loadFinalizer();

    assertNotNull(finalizerCopy);
    assertNotSame(Finalizer.class, finalizerCopy);

    assertNotNull(FinalizableReferenceQueue.getStartFinalizer(finalizerCopy));
  }

  static class DecoupledClassLoader extends URLClassLoader {

    public DecoupledClassLoader(URL[] urls) {
      super(urls);
    }

    @Override
    protected synchronized Class<?> loadClass(String name, boolean resolve)
        throws ClassNotFoundException {
            if (name.equals(Finalizer.class.getName())) {
        Class<?> clazz = findClass(name);
        if (resolve) {
          resolveClass(clazz);
        }
        return clazz;
      }

      return super.loadClass(name, resolve);
    }
  }

  @AndroidIncompatible   public void testGetFinalizerUrl() {
    assertNotNull(getClass().getResource("internal/Finalizer.class"));
  }

  public void testFinalizeClassHasNoNestedClasses() throws Exception {
            assertEquals(Collections.emptyList(), Arrays.asList(Finalizer.class.getDeclaredClasses()));
  }
}
