

package com.google.common.collect;

import java.util.Arrays;
import java.util.Map;
import java.util.Set;
import jsinterop.annotations.JsPackage;
import jsinterop.annotations.JsProperty;
import jsinterop.annotations.JsType;


final class Platform {
  static <K, V> Map<K, V> newHashMapWithExpectedSize(int expectedSize) {
    return Maps.newHashMapWithExpectedSize(expectedSize);
  }

  static <K, V> Map<K, V> newLinkedHashMapWithExpectedSize(int expectedSize) {
    return Maps.newLinkedHashMapWithExpectedSize(expectedSize);
  }

  static <E> Set<E> newHashSetWithExpectedSize(int expectedSize) {
    return Sets.newHashSetWithExpectedSize(expectedSize);
  }

  static <E> Set<E> newLinkedHashSetWithExpectedSize(int expectedSize) {
    return Sets.newLinkedHashSetWithExpectedSize(expectedSize);
  }

  
  static <K, V> Map<K, V> preservesInsertionOrderOnPutsMap() {
    return Maps.newLinkedHashMap();
  }

  
  static <E> Set<E> preservesInsertionOrderOnAddsSet() {
    return Sets.newLinkedHashSet();
  }

  static <T> T[] newArray(T[] reference, int length) {
    T[] clone = Arrays.copyOf(reference, 0);
    resizeArray(clone, length);
    return clone;
  }

  private static void resizeArray(Object array, int newSize) {
    ((NativeArray) array).setLength(newSize);
  }

    @JsType(isNative = true, name = "Array", namespace = JsPackage.GLOBAL)
  private interface NativeArray {
    @JsProperty
    void setLength(int length);
  }

  static MapMaker tryWeakKeys(MapMaker mapMaker) {
    return mapMaker;
  }

  static int reduceIterationsIfGwt(int iterations) {
    return iterations / 10;
  }

  static int reduceExponentIfGwt(int exponent) {
    return exponent / 2;
  }

  private Platform() {}
}
