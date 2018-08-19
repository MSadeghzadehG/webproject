
package com.iluwatar.throttling;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.iluwatar.throttling.timer.Throttler;

import java.util.concurrent.ThreadLocalRandom;


class B2BService {

  private static final Logger LOGGER = LoggerFactory.getLogger(B2BService.class);

  public B2BService(Throttler timer) {
    timer.start();
  }

  
  public int dummyCustomerApi(Tenant tenant) {
    String tenantName = tenant.getName();
    long count = CallsCount.getCount(tenantName);
    LOGGER.debug("Counter for {} : {} ", tenant.getName(), count);
    if (count >= tenant.getAllowedCallsPerSecond()) {
      LOGGER.error("API access per second limit reached for: {}", tenantName);
      return -1;
    }
    CallsCount.incrementCount(tenantName);
    return getRandomCustomerId();
  }

  private int getRandomCustomerId() {
    return ThreadLocalRandom.current().nextInt(1, 10000);
  }
}
