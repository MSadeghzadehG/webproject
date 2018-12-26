

package com.google.common.collect.testing.features;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.testing.Helpers;
import java.lang.annotation.Annotation;
import java.lang.reflect.AnnotatedElement;
import java.lang.reflect.Method;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Queue;
import java.util.Set;


@GwtIncompatible
public class FeatureUtil {
  
  private static Map<AnnotatedElement, List<Annotation>> annotationCache = new HashMap<>();

  private static final Map<Class<?>, TesterRequirements> classTesterRequirementsCache =
      new HashMap<>();

  private static final Map<Method, TesterRequirements> methodTesterRequirementsCache =
      new HashMap<>();

  
  public static Set<Feature<?>> addImpliedFeatures(Set<Feature<?>> features) {
    Queue<Feature<?>> queue = new ArrayDeque<>(features);
    while (!queue.isEmpty()) {
      Feature<?> feature = queue.remove();
      for (Feature<?> implied : feature.getImpliedFeatures()) {
        if (features.add(implied)) {
          queue.add(implied);
        }
      }
    }
    return features;
  }

  
  public static Set<Feature<?>> impliedFeatures(Set<Feature<?>> features) {
    Set<Feature<?>> impliedSet = new LinkedHashSet<>();
    Queue<Feature<?>> queue = new ArrayDeque<>(features);
    while (!queue.isEmpty()) {
      Feature<?> feature = queue.remove();
      for (Feature<?> implied : feature.getImpliedFeatures()) {
        if (!features.contains(implied) && impliedSet.add(implied)) {
          queue.add(implied);
        }
      }
    }
    return impliedSet;
  }

  
  public static TesterRequirements getTesterRequirements(Class<?> testerClass)
      throws ConflictingRequirementsException {
    synchronized (classTesterRequirementsCache) {
      TesterRequirements requirements = classTesterRequirementsCache.get(testerClass);
      if (requirements == null) {
        requirements = buildTesterRequirements(testerClass);
        classTesterRequirementsCache.put(testerClass, requirements);
      }
      return requirements;
    }
  }

  
  public static TesterRequirements getTesterRequirements(Method testerMethod)
      throws ConflictingRequirementsException {
    synchronized (methodTesterRequirementsCache) {
      TesterRequirements requirements = methodTesterRequirementsCache.get(testerMethod);
      if (requirements == null) {
        requirements = buildTesterRequirements(testerMethod);
        methodTesterRequirementsCache.put(testerMethod, requirements);
      }
      return requirements;
    }
  }

  
  static TesterRequirements buildTesterRequirements(Class<?> testerClass)
      throws ConflictingRequirementsException {
    final TesterRequirements declaredRequirements = buildDeclaredTesterRequirements(testerClass);
    Class<?> baseClass = testerClass.getSuperclass();
    if (baseClass == null) {
      return declaredRequirements;
    } else {
      final TesterRequirements clonedBaseRequirements =
          new TesterRequirements(getTesterRequirements(baseClass));
      return incorporateRequirements(clonedBaseRequirements, declaredRequirements, testerClass);
    }
  }

  
  static TesterRequirements buildTesterRequirements(Method testerMethod)
      throws ConflictingRequirementsException {
    TesterRequirements clonedClassRequirements =
        new TesterRequirements(getTesterRequirements(testerMethod.getDeclaringClass()));
    TesterRequirements declaredRequirements = buildDeclaredTesterRequirements(testerMethod);
    return incorporateRequirements(clonedClassRequirements, declaredRequirements, testerMethod);
  }

  
  private static TesterRequirements buildTesterRequirements(Annotation testerAnnotation)
      throws ConflictingRequirementsException {
    Class<? extends Annotation> annotationClass = testerAnnotation.annotationType();
    final Feature<?>[] presentFeatures;
    final Feature<?>[] absentFeatures;
    try {
      presentFeatures = (Feature[]) annotationClass.getMethod("value").invoke(testerAnnotation);
      absentFeatures = (Feature[]) annotationClass.getMethod("absent").invoke(testerAnnotation);
    } catch (Exception e) {
      throw new IllegalArgumentException("Error extracting features from tester annotation.", e);
    }
    Set<Feature<?>> allPresentFeatures =
        addImpliedFeatures(Helpers.<Feature<?>>copyToSet(presentFeatures));
    Set<Feature<?>> allAbsentFeatures =
        addImpliedFeatures(Helpers.<Feature<?>>copyToSet(absentFeatures));
    if (!Collections.disjoint(allPresentFeatures, allAbsentFeatures)) {
      throw new ConflictingRequirementsException(
          "Annotation explicitly or "
              + "implicitly requires one or more features to be both present "
              + "and absent.",
          intersection(allPresentFeatures, allAbsentFeatures),
          testerAnnotation);
    }
    return new TesterRequirements(allPresentFeatures, allAbsentFeatures);
  }

  
  public static TesterRequirements buildDeclaredTesterRequirements(AnnotatedElement classOrMethod)
      throws ConflictingRequirementsException {
    TesterRequirements requirements = new TesterRequirements();

    Iterable<Annotation> testerAnnotations = getTesterAnnotations(classOrMethod);
    for (Annotation testerAnnotation : testerAnnotations) {
      TesterRequirements moreRequirements = buildTesterRequirements(testerAnnotation);
      incorporateRequirements(requirements, moreRequirements, testerAnnotation);
    }

    return requirements;
  }

  
  public static Iterable<Annotation> getTesterAnnotations(AnnotatedElement classOrMethod) {
    synchronized (annotationCache) {
      List<Annotation> annotations = annotationCache.get(classOrMethod);
      if (annotations == null) {
        annotations = new ArrayList<>();
        for (Annotation a : classOrMethod.getDeclaredAnnotations()) {
          if (a.annotationType().isAnnotationPresent(TesterAnnotation.class)) {
            annotations.add(a);
          }
        }
        annotations = Collections.unmodifiableList(annotations);
        annotationCache.put(classOrMethod, annotations);
      }
      return annotations;
    }
  }

  
  private static TesterRequirements incorporateRequirements(
      TesterRequirements requirements, TesterRequirements moreRequirements, Object source)
      throws ConflictingRequirementsException {
    Set<Feature<?>> presentFeatures = requirements.getPresentFeatures();
    Set<Feature<?>> absentFeatures = requirements.getAbsentFeatures();
    Set<Feature<?>> morePresentFeatures = moreRequirements.getPresentFeatures();
    Set<Feature<?>> moreAbsentFeatures = moreRequirements.getAbsentFeatures();
    checkConflict("absent", absentFeatures, "present", morePresentFeatures, source);
    checkConflict("present", presentFeatures, "absent", moreAbsentFeatures, source);
    presentFeatures.addAll(morePresentFeatures);
    absentFeatures.addAll(moreAbsentFeatures);
    return requirements;
  }

    private static void checkConflict(
      String earlierRequirement,
      Set<Feature<?>> earlierFeatures,
      String newRequirement,
      Set<Feature<?>> newFeatures,
      Object source)
      throws ConflictingRequirementsException {
    if (!Collections.disjoint(newFeatures, earlierFeatures)) {
      throw new ConflictingRequirementsException(
          String.format(
              Locale.ROOT,
              "Annotation requires to be %s features that earlier "
                  + "annotations required to be %s.",
              newRequirement,
              earlierRequirement),
          intersection(newFeatures, earlierFeatures),
          source);
    }
  }

  
  public static <T> Set<T> intersection(Set<? extends T> set1, Set<? extends T> set2) {
    Set<T> result = Helpers.<T>copyToSet(set1);
    result.retainAll(set2);
    return result;
  }
}
