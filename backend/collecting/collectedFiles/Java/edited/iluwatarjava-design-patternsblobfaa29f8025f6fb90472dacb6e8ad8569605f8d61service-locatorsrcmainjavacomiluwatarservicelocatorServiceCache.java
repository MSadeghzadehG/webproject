
package com.iluwatar.servicelocator;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.HashMap;
import java.util.Map;


public class ServiceCache {

  private static final Logger LOGGER = LoggerFactory.getLogger(ServiceCache.class);

  private final Map<String, Service> serviceCache;

  public ServiceCache() {
    serviceCache = new HashMap<>();
  }

  
  public Service getService(String serviceName) {
    Service cachedService = null;
    for (String serviceJndiName : serviceCache.keySet()) {
      if (serviceJndiName.equals(serviceName)) {
        cachedService = serviceCache.get(serviceJndiName);
        LOGGER.info("(cache call) Fetched service {}({}) from cache... !",
                cachedService.getName(), cachedService.getId());
      }
    }
    return cachedService;
  }

  
  public void addService(Service newService) {
    serviceCache.put(newService.getName(), newService);
  }
}
