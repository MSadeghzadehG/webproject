

package com.google.common.collect.testing.features;

import java.lang.annotation.Annotation;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Method;
import java.util.Locale;
import junit.framework.TestCase;


public class FeatureEnumTest extends TestCase {
  private static void assertGoodTesterAnnotation(Class<? extends Annotation> annotationClass) {
    assertNotNull(
        rootLocaleFormat("%s must be annotated with @TesterAnnotation.", annotationClass),
        annotationClass.getAnnotation(TesterAnnotation.class));
    final Retention retentionPolicy = annotationClass.getAnnotation(Retention.class);
    assertNotNull(
        rootLocaleFormat("%s must have a @Retention annotation.", annotationClass),
        retentionPolicy);
    assertEquals(
        rootLocaleFormat("%s must have RUNTIME RetentionPolicy.", annotationClass),
        RetentionPolicy.RUNTIME,
        retentionPolicy.value());
    assertNotNull(
        rootLocaleFormat("%s must be inherited.", annotationClass),
        annotationClass.getAnnotation(Inherited.class));

    for (String propertyName : new String[] {"value", "absent"}) {
      Method method = null;
      try {
        method = annotationClass.getMethod(propertyName);
      } catch (NoSuchMethodException e) {
        fail(
            rootLocaleFormat("%s must have a property named '%s'.", annotationClass, propertyName));
      }
      final Class<?> returnType = method.getReturnType();
      assertTrue(
          rootLocaleFormat("%s.%s() must return an array.", annotationClass, propertyName),
          returnType.isArray());
      assertSame(
          rootLocaleFormat(
              "%s.%s() must return an array of %s.",
              annotationClass, propertyName, annotationClass.getDeclaringClass()),
          annotationClass.getDeclaringClass(),
          returnType.getComponentType());
    }
  }

      public static <E extends Enum<?> & Feature<?>> void assertGoodFeatureEnum(
      Class<E> featureEnumClass) {
    final Class<?>[] classes = featureEnumClass.getDeclaredClasses();
    for (Class<?> containedClass : classes) {
      if (containedClass.getSimpleName().equals("Require")) {
        if (containedClass.isAnnotation()) {
          assertGoodTesterAnnotation(asAnnotation(containedClass));
        } else {
          fail(
              rootLocaleFormat(
                  "Feature enum %s contains a class named "
                      + "'Require' but it is not an annotation.",
                  featureEnumClass));
        }
        return;
      }
    }
    fail(
        rootLocaleFormat(
            "Feature enum %s should contain an " + "annotation named 'Require'.",
            featureEnumClass));
  }

  @SuppressWarnings("unchecked")
  private static Class<? extends Annotation> asAnnotation(Class<?> clazz) {
    if (clazz.isAnnotation()) {
      return (Class<? extends Annotation>) clazz;
    } else {
      throw new IllegalArgumentException(rootLocaleFormat("%s is not an annotation.", clazz));
    }
  }

  public void testFeatureEnums() throws Exception {
    assertGoodFeatureEnum(CollectionFeature.class);
    assertGoodFeatureEnum(ListFeature.class);
    assertGoodFeatureEnum(SetFeature.class);
    assertGoodFeatureEnum(CollectionSize.class);
    assertGoodFeatureEnum(MapFeature.class);
  }

  private static String rootLocaleFormat(String format, Object... args) {
    return String.format(Locale.ROOT, format, args);
  }
}
