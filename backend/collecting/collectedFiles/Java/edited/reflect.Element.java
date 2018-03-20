

package com.google.common.reflect;

import static com.google.common.base.Preconditions.checkNotNull;

import java.lang.annotation.Annotation;
import java.lang.reflect.AccessibleObject;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Member;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


class Element extends AccessibleObject implements Member {

  private final AccessibleObject accessibleObject;
  private final Member member;

  <M extends AccessibleObject & Member> Element(M member) {
    checkNotNull(member);
    this.accessibleObject = member;
    this.member = member;
  }

  public TypeToken<?> getOwnerType() {
    return TypeToken.of(getDeclaringClass());
  }

  @Override
  public final boolean isAnnotationPresent(Class<? extends Annotation> annotationClass) {
    return accessibleObject.isAnnotationPresent(annotationClass);
  }

  @Override
  public final <A extends Annotation> A getAnnotation(Class<A> annotationClass) {
    return accessibleObject.getAnnotation(annotationClass);
  }

  @Override
  public final Annotation[] getAnnotations() {
    return accessibleObject.getAnnotations();
  }

  @Override
  public final Annotation[] getDeclaredAnnotations() {
    return accessibleObject.getDeclaredAnnotations();
  }

  @Override
  public final void setAccessible(boolean flag) throws SecurityException {
    accessibleObject.setAccessible(flag);
  }

  @Override
  public final boolean isAccessible() {
    return accessibleObject.isAccessible();
  }

  @Override
  public Class<?> getDeclaringClass() {
    return member.getDeclaringClass();
  }

  @Override
  public final String getName() {
    return member.getName();
  }

  @Override
  public final int getModifiers() {
    return member.getModifiers();
  }

  @Override
  public final boolean isSynthetic() {
    return member.isSynthetic();
  }

  
  public final boolean isPublic() {
    return Modifier.isPublic(getModifiers());
  }

  
  public final boolean isProtected() {
    return Modifier.isProtected(getModifiers());
  }

  
  public final boolean isPackagePrivate() {
    return !isPrivate() && !isPublic() && !isProtected();
  }

  
  public final boolean isPrivate() {
    return Modifier.isPrivate(getModifiers());
  }

  
  public final boolean isStatic() {
    return Modifier.isStatic(getModifiers());
  }

  
  public final boolean isFinal() {
    return Modifier.isFinal(getModifiers());
  }

  
  public final boolean isAbstract() {
    return Modifier.isAbstract(getModifiers());
  }

  
  public final boolean isNative() {
    return Modifier.isNative(getModifiers());
  }

  
  public final boolean isSynchronized() {
    return Modifier.isSynchronized(getModifiers());
  }

  
  final boolean isVolatile() {
    return Modifier.isVolatile(getModifiers());
  }

  
  final boolean isTransient() {
    return Modifier.isTransient(getModifiers());
  }

  @Override
  public boolean equals(@NullableDecl Object obj) {
    if (obj instanceof Element) {
      Element that = (Element) obj;
      return getOwnerType().equals(that.getOwnerType()) && member.equals(that.member);
    }
    return false;
  }

  @Override
  public int hashCode() {
    return member.hashCode();
  }

  @Override
  public String toString() {
    return member.toString();
  }
}
