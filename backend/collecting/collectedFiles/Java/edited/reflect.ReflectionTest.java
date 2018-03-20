

package com.google.common.reflect;

import com.google.common.testing.NullPointerTester;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.util.Map;
import junit.framework.TestCase;


public class ReflectionTest extends TestCase {

  public void testGetPackageName() throws Exception {
    assertEquals("java.lang", Reflection.getPackageName(Iterable.class));
    assertEquals("java", Reflection.getPackageName("java.MyType"));
    assertEquals("java.lang", Reflection.getPackageName(Iterable.class.getName()));
    assertEquals("", Reflection.getPackageName("NoPackage"));
    assertEquals("java.util", Reflection.getPackageName(Map.Entry.class));
  }

  public void testNewProxy() throws Exception {
    Runnable runnable = Reflection.newProxy(Runnable.class, X_RETURNER);
    assertEquals("x", runnable.toString());
  }

  public void testNewProxyCantWorkOnAClass() throws Exception {
    try {
      Reflection.newProxy(Object.class, X_RETURNER);
      fail();
    } catch (IllegalArgumentException expected) {
    }
  }

  private static final InvocationHandler X_RETURNER =
      new InvocationHandler() {
        @Override
        public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
          return "x";
        }
      };

  private static int classesInitialized = 0;

  private static class A {
    static {
      ++classesInitialized;
    }
  }

  private static class B {
    static {
      ++classesInitialized;
    }
  }

  private static class C {
    static {
      ++classesInitialized;
    }
  }

  public void testInitialize() {
    assertEquals("This test can't be included twice in the same suite.", 0, classesInitialized);

    Reflection.initialize(A.class);
    assertEquals(1, classesInitialized);

    Reflection.initialize(
        A.class,         B.class, C.class);
    assertEquals(3, classesInitialized);
  }

  public void testNullPointers() {
    new NullPointerTester().testAllPublicStaticMethods(Reflection.class);
  }
}
