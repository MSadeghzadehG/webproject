

package com.google.common.base;

import static com.google.common.base.StandardSystemProperty.JAVA_CLASS_PATH;
import static com.google.common.base.StandardSystemProperty.PATH_SEPARATOR;

import com.google.common.collect.ImmutableList;
import com.google.common.testing.GcFinalization;
import java.io.Closeable;
import java.io.File;
import java.lang.ref.WeakReference;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.security.Permission;
import java.security.Policy;
import java.security.ProtectionDomain;
import java.util.concurrent.Callable;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;
import junit.framework.TestCase;


public class FinalizableReferenceQueueClassLoaderUnloadingTest extends TestCase {

  

  public static class MyFinalizableWeakReference extends FinalizableWeakReference<Object> {
    public MyFinalizableWeakReference(Object x, FinalizableReferenceQueue queue) {
      super(x, queue);
    }

    @Override
    public void finalizeReferent() {}
  }

  private static class PermissivePolicy extends Policy {
    @Override
    public boolean implies(ProtectionDomain pd, Permission perm) {
      return true;
    }
  }

  private WeakReference<ClassLoader> useFrqInSeparateLoader() throws Exception {
    final ClassLoader myLoader = getClass().getClassLoader();
    URLClassLoader sepLoader = new URLClassLoader(getClassPathUrls(), myLoader.getParent());
                    
    Class<?> frqC = FinalizableReferenceQueue.class;
    Class<?> sepFrqC = sepLoader.loadClass(frqC.getName());
    assertNotSame(frqC, sepFrqC);
    
                            Class<?> sepFrqSystemLoaderC =
        sepLoader.loadClass(FinalizableReferenceQueue.SystemLoader.class.getName());
    Field disabled = sepFrqSystemLoaderC.getDeclaredField("disabled");
    disabled.setAccessible(true);
    disabled.set(null, true);

            AtomicReference<Object> sepFrqA = new AtomicReference<Object>(sepFrqC.newInstance());
    Class<?> sepFwrC = sepLoader.loadClass(MyFinalizableWeakReference.class.getName());
    Constructor<?> sepFwrCons = sepFwrC.getConstructor(Object.class, sepFrqC);
        Class<?> sepStopwatchC = sepLoader.loadClass(Stopwatch.class.getName());
    assertSame(sepLoader, sepStopwatchC.getClassLoader());
    AtomicReference<Object> sepStopwatchA =
        new AtomicReference<Object>(sepStopwatchC.getMethod("createUnstarted").invoke(null));
    AtomicReference<WeakReference<?>> sepStopwatchRef =
        new AtomicReference<WeakReference<?>>(
            (WeakReference<?>) sepFwrCons.newInstance(sepStopwatchA.get(), sepFrqA.get()));
    assertNotNull(sepStopwatchA.get());
        sepStopwatchA.set(null);
    GcFinalization.awaitClear(sepStopwatchRef.get());
            return new WeakReference<ClassLoader>(sepLoader);
  }

  private void doTestUnloadable() throws Exception {
    WeakReference<ClassLoader> loaderRef = useFrqInSeparateLoader();
    GcFinalization.awaitClear(loaderRef);
  }

  public void testUnloadableWithoutSecurityManager() throws Exception {
            SecurityManager oldSecurityManager = System.getSecurityManager();
    try {
      System.setSecurityManager(null);
      doTestUnloadable();
    } finally {
      System.setSecurityManager(oldSecurityManager);
    }
  }

  public void testUnloadableWithSecurityManager() throws Exception {
                        Policy oldPolicy = Policy.getPolicy();
    SecurityManager oldSecurityManager = System.getSecurityManager();
    try {
      Policy.setPolicy(new PermissivePolicy());
      System.setSecurityManager(new SecurityManager());
      doTestUnloadable();
    } finally {
      System.setSecurityManager(oldSecurityManager);
      Policy.setPolicy(oldPolicy);
    }
  }

  public static class FrqUser implements Callable<WeakReference<Object>> {
    public static FinalizableReferenceQueue frq = new FinalizableReferenceQueue();
    public static final Semaphore finalized = new Semaphore(0);

    @Override
    public WeakReference<Object> call() {
      WeakReference<Object> wr =
          new FinalizableWeakReference<Object>(new Integer(23), frq) {
            @Override
            public void finalizeReferent() {
              finalized.release();
            }
          };
      return wr;
    }
  }

  public void testUnloadableInStaticFieldIfClosed() throws Exception {
    Policy oldPolicy = Policy.getPolicy();
    SecurityManager oldSecurityManager = System.getSecurityManager();
    try {
      Policy.setPolicy(new PermissivePolicy());
      System.setSecurityManager(new SecurityManager());
      WeakReference<ClassLoader> loaderRef = doTestUnloadableInStaticFieldIfClosed();
      GcFinalization.awaitClear(loaderRef);
    } finally {
      System.setSecurityManager(oldSecurityManager);
      Policy.setPolicy(oldPolicy);
    }
  }

                        private WeakReference<ClassLoader> doTestUnloadableInStaticFieldIfClosed() throws Exception {
    final ClassLoader myLoader = getClass().getClassLoader();
    URLClassLoader sepLoader = new URLClassLoader(getClassPathUrls(), myLoader.getParent());

    Class<?> frqC = FinalizableReferenceQueue.class;
    Class<?> sepFrqC = sepLoader.loadClass(frqC.getName());
    assertNotSame(frqC, sepFrqC);

    Class<?> sepFrqSystemLoaderC =
        sepLoader.loadClass(FinalizableReferenceQueue.SystemLoader.class.getName());
    Field disabled = sepFrqSystemLoaderC.getDeclaredField("disabled");
    disabled.setAccessible(true);
    disabled.set(null, true);

    Class<?> frqUserC = FrqUser.class;
    Class<?> sepFrqUserC = sepLoader.loadClass(frqUserC.getName());
    assertNotSame(frqUserC, sepFrqUserC);
    assertSame(sepLoader, sepFrqUserC.getClassLoader());

    Callable<?> sepFrqUser = (Callable<?>) sepFrqUserC.newInstance();
    WeakReference<?> finalizableWeakReference = (WeakReference<?>) sepFrqUser.call();

    GcFinalization.awaitClear(finalizableWeakReference);

    Field sepFrqUserFinalizedF = sepFrqUserC.getField("finalized");
    Semaphore finalizeCount = (Semaphore) sepFrqUserFinalizedF.get(null);
    boolean finalized = finalizeCount.tryAcquire(5, TimeUnit.SECONDS);
    assertTrue(finalized);

    Field sepFrqUserFrqF = sepFrqUserC.getField("frq");
    Closeable frq = (Closeable) sepFrqUserFrqF.get(null);
    frq.close();

    return new WeakReference<ClassLoader>(sepLoader);
  }

  private URL[] getClassPathUrls() {
    ClassLoader classLoader = getClass().getClassLoader();
    return classLoader instanceof URLClassLoader
        ? ((URLClassLoader) classLoader).getURLs()
        : parseJavaClassPath().toArray(new URL[0]);
  }

  
    private static ImmutableList<URL> parseJavaClassPath() {
    ImmutableList.Builder<URL> urls = ImmutableList.builder();
    for (String entry : Splitter.on(PATH_SEPARATOR.value()).split(JAVA_CLASS_PATH.value())) {
      try {
        try {
          urls.add(new File(entry).toURI().toURL());
        } catch (SecurityException e) {           urls.add(new URL("file", null, new File(entry).getAbsolutePath()));
        }
      } catch (MalformedURLException e) {
        AssertionError error = new AssertionError("malformed class path entry: " + entry);
        error.initCause(e);
        throw error;
      }
    }
    return urls.build();
  }
}
