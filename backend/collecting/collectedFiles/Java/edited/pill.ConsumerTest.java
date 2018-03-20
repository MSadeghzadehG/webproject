
package com.iluwatar.poison.pill;

import ch.qos.logback.classic.Logger;
import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.AppenderBase;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.slf4j.LoggerFactory;

import java.time.LocalDateTime;
import java.util.LinkedList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertTrue;


public class ConsumerTest {

  private InMemoryAppender appender;

  @BeforeEach
  public void setUp() {
    appender = new InMemoryAppender(Consumer.class);
  }

  @AfterEach
  public void tearDown() {
    appender.stop();
  }

  @Test
  public void testConsume() throws Exception {
    final Message[] messages = new Message[]{
        createMessage("you", "Hello!"),
        createMessage("me", "Hi!"),
        Message.POISON_PILL,
        createMessage("late_for_the_party", "Hello? Anyone here?"),
    };

    final MessageQueue queue = new SimpleMessageQueue(messages.length);
    for (final Message message : messages) {
      queue.put(message);
    }

    new Consumer("NSA", queue).consume();

    assertTrue(appender.logContains("Message [Hello!] from [you] received by [NSA]"));
    assertTrue(appender.logContains("Message [Hi!] from [me] received by [NSA]"));
    assertTrue(appender.logContains("Consumer NSA receive request to terminate."));
  }

  
  private static Message createMessage(final String sender, final String message) {
    final SimpleMessage msg = new SimpleMessage();
    msg.addHeader(Message.Headers.SENDER, sender);
    msg.addHeader(Message.Headers.DATE, LocalDateTime.now().toString());
    msg.setBody(message);
    return msg;
  }

  private class InMemoryAppender extends AppenderBase<ILoggingEvent> {
    private List<ILoggingEvent> log = new LinkedList<>();

    public InMemoryAppender(Class clazz) {
      ((Logger) LoggerFactory.getLogger(clazz)).addAppender(this);
      start();
    }

    @Override
    protected void append(ILoggingEvent eventObject) {
      log.add(eventObject);
    }

    public boolean logContains(String message) {
      return log.stream().anyMatch(event -> event.getFormattedMessage().equals(message));
    }
  }

}
