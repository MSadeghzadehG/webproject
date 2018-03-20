
package com.iluwatar.databus.members;

import com.iluwatar.databus.data.MessageData;
import com.iluwatar.databus.data.StartingData;
import org.junit.jupiter.api.Test;

import java.time.LocalDateTime;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;


public class MessageCollectorMemberTest {

  @Test
  public void collectMessageFromMessageData() {
        final String message = "message";
    final MessageData messageData = new MessageData(message);
    final MessageCollectorMember collector = new MessageCollectorMember("collector");
        collector.accept(messageData);
        assertTrue(collector.getMessages().contains(message));
  }

  @Test
  public void collectIgnoresMessageFromOtherDataTypes() {
        final StartingData startingData = new StartingData(LocalDateTime.now());
    final MessageCollectorMember collector = new MessageCollectorMember("collector");
        collector.accept(startingData);
        assertEquals(0, collector.getMessages().size());
  }

}
