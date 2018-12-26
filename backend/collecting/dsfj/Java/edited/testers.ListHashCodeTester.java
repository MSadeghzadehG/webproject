

package com.google.common.collect.testing.testers;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.Helpers;
import java.lang.reflect.Method;
import org.junit.Ignore;


@GwtCompatible(emulated = true)
@Ignore public class ListHashCodeTester<E> extends AbstractListTester<E> {
  public void testHashCode() {
    int expectedHashCode = 1;
    for (E element : getOrderedElements()) {
      expectedHashCode = 31 * expectedHashCode + ((element == null) ? 0 : element.hashCode());
    }
    assertEquals(
        "A List's hashCode() should be computed from those of its elements.",
        expectedHashCode,
        getList().hashCode());
  }

  
  @GwtIncompatible   public static Method getHashCodeMethod() {
    return Helpers.getMethod(ListHashCodeTester.class, "testHashCode");
  }
}
