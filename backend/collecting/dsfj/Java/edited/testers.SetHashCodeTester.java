

package com.google.common.collect.testing.testers;

import static com.google.common.collect.testing.features.CollectionFeature.ALLOWS_NULL_VALUES;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.Helpers;
import com.google.common.collect.testing.features.CollectionFeature;
import com.google.common.collect.testing.features.CollectionSize;
import java.lang.reflect.Method;
import java.util.Collection;
import org.junit.Ignore;


@GwtCompatible(emulated = true)
@Ignore public class SetHashCodeTester<E> extends AbstractSetTester<E> {
  public void testHashCode() {
    int expectedHashCode = 0;
    for (E element : getSampleElements()) {
      expectedHashCode += ((element == null) ? 0 : element.hashCode());
    }
    assertEquals(
        "A Set's hashCode() should be the sum of those of its elements.",
        expectedHashCode,
        getSet().hashCode());
  }

  @CollectionSize.Require(absent = CollectionSize.ZERO)
  @CollectionFeature.Require(ALLOWS_NULL_VALUES)
  public void testHashCode_containingNull() {
    Collection<E> elements = getSampleElements(getNumElements() - 1);
    int expectedHashCode = 0;
    for (E element : elements) {
      expectedHashCode += ((element == null) ? 0 : element.hashCode());
    }

    elements.add(null);
    collection = getSubjectGenerator().create(elements.toArray());
    assertEquals(
        "A Set's hashCode() should be the sum of those of its elements (with "
            + "a null element counting as having a hash of zero).",
        expectedHashCode,
        getSet().hashCode());
  }

  
  @GwtIncompatible   public static Method[] getHashCodeMethods() {
    return new Method[] {
      Helpers.getMethod(SetHashCodeTester.class, "testHashCode"),
      Helpers.getMethod(SetHashCodeTester.class, "testHashCode_containingNull")
    };
  }
}
