

package com.google.common.reflect;

import static com.google.common.base.Preconditions.checkState;
import static com.google.common.truth.Truth.assertThat;

import com.google.errorprone.annotations.RequiredModifiers;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.lang.reflect.Method;
import java.lang.reflect.Type;
import java.util.Arrays;
import java.util.Comparator;
import javax.lang.model.element.Modifier;


@AndroidIncompatible abstract class SubtypeTester implements Cloneable {

  
  @RequiredModifiers(Modifier.PUBLIC)
  @Retention(RetentionPolicy.RUNTIME)
  @Target(ElementType.METHOD)
  @interface TestSubtype {
    
    boolean suppressGetSubtype() default false;

    
    boolean suppressGetSupertype() default false;
  }

  private Method method = null;

  
  final <T> T isSubtype(T sub) {
    Type returnType = method.getGenericReturnType();
    Type paramType = getOnlyParameterType();
    TestSubtype spec = method.getAnnotation(TestSubtype.class);
    assertThat(TypeToken.of(paramType).isSubtypeOf(returnType))
        .named("%s is subtype of %s", paramType, returnType)
        .isTrue();
    assertThat(TypeToken.of(returnType).isSupertypeOf(paramType))
        .named("%s is supertype of %s", returnType, paramType)
        .isTrue();
    if (!spec.suppressGetSubtype()) {
      assertThat(getSubtype(returnType, TypeToken.of(paramType).getRawType())).isEqualTo(paramType);
    }
    if (!spec.suppressGetSupertype()) {
      assertThat(getSupertype(paramType, TypeToken.of(returnType).getRawType()))
          .isEqualTo(returnType);
    }
    return sub;
  }

  
  final <X> X notSubtype(@SuppressWarnings("unused") Object sub) {
    Type returnType = method.getGenericReturnType();
    Type paramType = getOnlyParameterType();
    TestSubtype spec = method.getAnnotation(TestSubtype.class);
    assertThat(TypeToken.of(paramType).isSubtypeOf(returnType))
        .named("%s is subtype of %s", paramType, returnType)
        .isFalse();
    assertThat(TypeToken.of(returnType).isSupertypeOf(paramType))
        .named("%s is supertype of %s", returnType, paramType)
        .isFalse();
    if (!spec.suppressGetSubtype()) {
      try {
        assertThat(getSubtype(returnType, TypeToken.of(paramType).getRawType()))
            .isNotEqualTo(paramType);
      } catch (IllegalArgumentException notSubtype1) {
              }
    }
    if (!spec.suppressGetSupertype()) {
      try {
        assertThat(getSupertype(paramType, TypeToken.of(returnType).getRawType()))
            .isNotEqualTo(returnType);
      } catch (IllegalArgumentException notSubtype2) {
              }
    }
    return null;
  }

  final void testAllDeclarations() throws Exception {
    checkState(method == null);
    Method[] methods = getClass().getMethods();
    Arrays.sort(
        methods,
        new Comparator<Method>() {
          @Override
          public int compare(Method a, Method b) {
            return a.getName().compareTo(b.getName());
          }
        });
    for (Method method : methods) {
      if (method.isAnnotationPresent(TestSubtype.class)) {
        method.setAccessible(true);
        SubtypeTester tester = (SubtypeTester) clone();
        tester.method = method;
        method.invoke(tester, new Object[] {null});
      }
    }
  }

  private Type getOnlyParameterType() {
    assertThat(method.getGenericParameterTypes()).hasLength(1);
    return method.getGenericParameterTypes()[0];
  }

  @SuppressWarnings({"rawtypes", "unchecked"})
  private static Type getSupertype(Type type, Class<?> superclass) {
    Class rawType = superclass;
    return TypeToken.of(type).getSupertype(rawType).getType();
  }

  private static Type getSubtype(Type type, Class<?> subclass) {
    return TypeToken.of(type).getSubtype(subclass).getType();
  }
}
