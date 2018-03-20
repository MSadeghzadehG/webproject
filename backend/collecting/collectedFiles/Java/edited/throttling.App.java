

package com.iluwatar.throttling;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.iluwatar.throttling.timer.Throttler;
import com.iluwatar.throttling.timer.ThrottleTimerImpl;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) {

    Tenant adidas = new Tenant("Adidas", 5);
    Tenant nike = new Tenant("Nike", 6);

    ExecutorService executorService = Executors.newFixedThreadPool(2);
    
    executorService.execute(() -> makeServiceCalls(adidas));
    executorService.execute(() -> makeServiceCalls(nike));
    
    executorService.shutdown();
    try {
      executorService.awaitTermination(10, TimeUnit.SECONDS);
    } catch (InterruptedException e) {
      LOGGER.error("Executor Service terminated: {}", e.getMessage());
    }
  }

  
  private static void makeServiceCalls(Tenant tenant) {
    Throttler timer = new ThrottleTimerImpl(10);
    B2BService service = new B2BService(timer);
    for (int i = 0; i < 20; i++) {
      service.dummyCustomerApi(tenant);
      try {
        Thread.sleep(1);
      } catch (InterruptedException e) {
        LOGGER.error("Thread interrupted: {}", e.getMessage());
      }
    }
  }
}
