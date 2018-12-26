

package com.google.common.base;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.annotations.VisibleForTesting;
import java.io.Closeable;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.ref.PhantomReference;
import java.lang.ref.Reference;
import java.lang.ref.ReferenceQueue;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtIncompatible
public class FinalizableReferenceQueue implements Closeable {
  

  private static final Logger logger = Logger.getLogger(FinalizableReferenceQueue.class.getName());

  private static final String FINALIZER_CLASS_NAME = "com.google.common.base.internal.Finalizer";

  
  private static final Method startFinalizer;

  static {
    Class<?> finalizer =
        loadFinalizer(new SystemLoader(), new DecoupledLoader(), new DirectLoader());
    startFinalizer = getStartFinalizer(finalizer);
  }

  
  final ReferenceQueue<Object> queue;

  final PhantomReference<Object> frqRef;

  
  final boolean threadStarted;

  
  public FinalizableReferenceQueue() {
        queue = new ReferenceQueue<>();
    frqRef = new PhantomReference<Object>(this, queue);
    boolean threadStarted = false;
    try {
      startFinalizer.invoke(null, FinalizableReference.class, queue, frqRef);
      threadStarted = true;
    } catch (IllegalAccessException impossible) {
      throw new AssertionError(impossible);     } catch (Throwable t) {
      logger.log(
          Level.INFO,
          "Failed to start reference finalizer thread."
              + " Reference cleanup will only occur when new references are created.",
          t);
    }

    this.threadStarted = threadStarted;
  }

  @Override
  public void close() {
    frqRef.enqueue();
    cleanUp();
  }

  
  void cleanUp() {
    if (threadStarted) {
      return;
    }

    Reference<?> reference;
    while ((reference = queue.poll()) != null) {
      
      reference.clear();
      try {
        ((FinalizableReference) reference).finalizeReferent();
      } catch (Throwable t) {
        logger.log(Level.SEVERE, "Error cleaning up after reference.", t);
      }
    }
  }

  
  private static Class<?> loadFinalizer(FinalizerLoader... loaders) {
    for (FinalizerLoader loader : loaders) {
      Class<?> finalizer = loader.loadFinalizer();
      if (finalizer != null) {
        return finalizer;
      }
    }

    throw new AssertionError();
  }

  
  interface FinalizerLoader {

    
    @NullableDecl
    Class<?> loadFinalizer();
  }

  
  static class SystemLoader implements FinalizerLoader {
            @VisibleForTesting static boolean disabled;

    @NullableDecl
    @Override
    public Class<?> loadFinalizer() {
      if (disabled) {
        return null;
      }
      ClassLoader systemLoader;
      try {
        systemLoader = ClassLoader.getSystemClassLoader();
      } catch (SecurityException e) {
        logger.info("Not allowed to access system class loader.");
        return null;
      }
      if (systemLoader != null) {
        try {
          return systemLoader.loadClass(FINALIZER_CLASS_NAME);
        } catch (ClassNotFoundException e) {
                    return null;
        }
      } else {
        return null;
      }
    }
  }

  
  static class DecoupledLoader implements FinalizerLoader {
    private static final String LOADING_ERROR =
        "Could not load Finalizer in its own class loader. Loading Finalizer in the current class "
            + "loader instead. As a result, you will not be able to garbage collect this class "
            + "loader. To support reclaiming this class loader, either resolve the underlying "
            + "issue, or move Guava to your system class path.";

    @NullableDecl
    @Override
    public Class<?> loadFinalizer() {
      try {
        
        ClassLoader finalizerLoader = newLoader(getBaseUrl());
        return finalizerLoader.loadClass(FINALIZER_CLASS_NAME);
      } catch (Exception e) {
        logger.log(Level.WARNING, LOADING_ERROR, e);
        return null;
      }
    }

    
    URL getBaseUrl() throws IOException {
            String finalizerPath = FINALIZER_CLASS_NAME.replace('.', '/') + ".class";
      URL finalizerUrl = getClass().getClassLoader().getResource(finalizerPath);
      if (finalizerUrl == null) {
        throw new FileNotFoundException(finalizerPath);
      }

            String urlString = finalizerUrl.toString();
      if (!urlString.endsWith(finalizerPath)) {
        throw new IOException("Unsupported path style: " + urlString);
      }
      urlString = urlString.substring(0, urlString.length() - finalizerPath.length());
      return new URL(finalizerUrl, urlString);
    }

    
    URLClassLoader newLoader(URL base) {
                        return new URLClassLoader(new URL[] {base}, null);
    }
  }

  
  static class DirectLoader implements FinalizerLoader {
    @Override
    public Class<?> loadFinalizer() {
      try {
        return Class.forName(FINALIZER_CLASS_NAME);
      } catch (ClassNotFoundException e) {
        throw new AssertionError(e);
      }
    }
  }

  
  static Method getStartFinalizer(Class<?> finalizer) {
    try {
      return finalizer.getMethod(
          "startFinalizer", Class.class, ReferenceQueue.class, PhantomReference.class);
    } catch (NoSuchMethodException e) {
      throw new AssertionError(e);
    }
  }
}
