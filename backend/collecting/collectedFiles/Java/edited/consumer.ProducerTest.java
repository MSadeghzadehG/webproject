
package com.iluwatar.producer.consumer;

import org.junit.jupiter.api.Test;

import static java.time.Duration.ofMillis;
import static org.junit.jupiter.api.Assertions.assertTimeout;
import static org.mockito.Matchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;


public class ProducerTest {

  @Test
  public void testProduce() throws Exception {
    assertTimeout(ofMillis(6000), () -> {
      final ItemQueue queue = mock(ItemQueue.class);
      final Producer producer = new Producer("producer", queue);

      producer.produce();
      verify(queue).put(any(Item.class));

      verifyNoMoreInteractions(queue);
    });
  }

}