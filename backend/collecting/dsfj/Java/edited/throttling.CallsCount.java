
package com.iluwatar.throttling;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;


public final class CallsCount {

  private static final Logger LOGGER = LoggerFactory.getLogger(CallsCount.class);
  private static Map<String, AtomicLong> tenantCallsCount = new ConcurrentHashMap<>();

  
  public static void addTenant(String tenantName) {
    tenantCallsCount.putIfAbsent(tenantName, new AtomicLong(0));
  }
  
  
  public static void incrementCount(String tenantName) {
    tenantCallsCount.get(tenantName).incrementAndGet();
  }
  
  
  public static long getCount(String tenantName) {
    return tenantCallsCount.get(tenantName).get();
  }
  
  
  public static void reset() {
    LOGGER.debug("Resetting the map.");
    for (Entry<String, AtomicLong> e : tenantCallsCount.entrySet()) {
      tenantCallsCount.put(e.getKey(), new AtomicLong(0));
    }
  }
}
