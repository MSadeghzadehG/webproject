

package com.google.common.eventbus.outside;

import com.google.common.eventbus.EventBus;
import com.google.common.eventbus.Subscribe;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;
import junit.framework.TestCase;


public class OutsideEventBusTest extends TestCase {

  
  public void testAnonymous() {
    final AtomicReference<String> holder = new AtomicReference<>();
    final AtomicInteger deliveries = new AtomicInteger();
    EventBus bus = new EventBus();
    bus.register(
        new Object() {
          @Subscribe
          public void accept(String str) {
            holder.set(str);
            deliveries.incrementAndGet();
          }
        });

    String EVENT = "Hello!";
    bus.post(EVENT);

    assertEquals("Only one event should be delivered.", 1, deliveries.get());
    assertEquals("Correct string should be delivered.", EVENT, holder.get());
  }
}
