

package com.google.zxing.web;

import java.util.LinkedHashMap;
import java.util.Map;


final class LRUMap<K,V> extends LinkedHashMap<K,V> {

  private final int maxSize;

  LRUMap(int maxSize) {
    super(100, 0.75f, true);
    this.maxSize = maxSize;
  }

  @Override
  protected boolean removeEldestEntry(Map.Entry<K,V> eldest) {
    return size() > maxSize;
  }

}
