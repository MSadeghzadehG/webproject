
package com.iluwatar.databus.members;

import com.iluwatar.databus.DataBus;
import com.iluwatar.databus.data.MessageData;
import com.iluwatar.databus.data.StartingData;
import com.iluwatar.databus.data.StoppingData;
import org.junit.jupiter.api.Test;

import java.time.LocalDateTime;
import java.time.Month;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNull;


public class StatusMemberTest {

  @Test
  public void statusRecordsTheStartTime() {
        final LocalDateTime startTime = LocalDateTime.of(2017, Month.APRIL, 1, 19, 9);
    final StartingData startingData = new StartingData(startTime);
    final StatusMember statusMember = new StatusMember(1);
        statusMember.accept(startingData);
        assertEquals(startTime, statusMember.getStarted());
  }

  @Test
  public void statusRecordsTheStopTime() {
        final LocalDateTime stop = LocalDateTime.of(2017, Month.APRIL, 1, 19, 12);
    final StoppingData stoppingData = new StoppingData(stop);
    stoppingData.setDataBus(DataBus.getInstance());
    final StatusMember statusMember = new StatusMember(1);
        statusMember.accept(stoppingData);
        assertEquals(stop, statusMember.getStopped());
  }

  @Test
  public void statusIgnoresMessageData() {
        final MessageData messageData = new MessageData("message");
    final StatusMember statusMember = new StatusMember(1);
        statusMember.accept(messageData);
        assertNull(statusMember.getStarted());
    assertNull(statusMember.getStopped());
  }

}
