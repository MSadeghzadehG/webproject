
package com.iluwatar.poison.pill;

import org.junit.jupiter.api.Test;

import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;


public class SimpleMessageTest {

  @Test
  public void testGetHeaders() {
    final SimpleMessage message = new SimpleMessage();
    assertNotNull(message.getHeaders());
    assertTrue(message.getHeaders().isEmpty());

    final String senderName = "test";
    message.addHeader(Message.Headers.SENDER, senderName);
    assertNotNull(message.getHeaders());
    assertFalse(message.getHeaders().isEmpty());
    assertEquals(senderName, message.getHeaders().get(Message.Headers.SENDER));
  }

  @Test
  public void testUnModifiableHeaders() {
    final SimpleMessage message = new SimpleMessage();
    final Map<Message.Headers, String> headers = message.getHeaders();
    assertThrows(UnsupportedOperationException.class, () -> {
      headers.put(Message.Headers.SENDER, "test");
    });
  }


}