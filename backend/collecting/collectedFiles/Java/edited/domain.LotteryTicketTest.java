
package com.iluwatar.hexagonal.domain;

import org.junit.jupiter.api.Test;

import java.util.Arrays;
import java.util.HashSet;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;


public class LotteryTicketTest {

  @Test
  public void testEquals() {
    PlayerDetails details1 = new PlayerDetails("bob@foo.bar", "1212-121212", "+34332322");
    LotteryNumbers numbers1 = LotteryNumbers.create(new HashSet<Integer>(Arrays.asList(1, 2, 3, 4)));
    LotteryTicket ticket1 = new LotteryTicket(new LotteryTicketId(), details1, numbers1);
    PlayerDetails details2 = new PlayerDetails("bob@foo.bar", "1212-121212", "+34332322");
    LotteryNumbers numbers2 = LotteryNumbers.create(new HashSet<Integer>(Arrays.asList(1, 2, 3, 4)));
    LotteryTicket ticket2 = new LotteryTicket(new LotteryTicketId(), details2, numbers2);
    assertEquals(ticket1, ticket2);
    PlayerDetails details3 = new PlayerDetails("elsa@foo.bar", "1223-121212", "+49332322");
    LotteryNumbers numbers3 = LotteryNumbers.create(new HashSet<Integer>(Arrays.asList(1, 2, 3, 8)));
    LotteryTicket ticket3 = new LotteryTicket(new LotteryTicketId(), details3, numbers3);
    assertFalse(ticket1.equals(ticket3));
  }
}
