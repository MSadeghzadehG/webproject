
package com.iluwatar.hexagonal.domain;

import com.google.inject.Guice;
import com.google.inject.Inject;
import com.google.inject.Injector;
import com.iluwatar.hexagonal.banking.WireTransfers;
import com.iluwatar.hexagonal.domain.LotteryTicketCheckResult.CheckResult;
import com.iluwatar.hexagonal.module.LotteryTestingModule;
import com.iluwatar.hexagonal.test.LotteryTestUtils;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Map;
import java.util.Optional;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;


public class LotteryTest {

  private Injector injector;
  @Inject
  private LotteryAdministration administration;
  @Inject
  private LotteryService service;
  @Inject
  private WireTransfers wireTransfers;

  public LotteryTest() {
    this.injector = Guice.createInjector(new LotteryTestingModule());
  }

  @BeforeEach
  public void setup() {
    injector.injectMembers(this);
        wireTransfers.setFunds("123-12312", 100);
  }
  
  @Test
  public void testLottery() {
        administration.resetLottery();
    assertEquals(administration.getAllSubmittedTickets().size(), 0);
    
        Optional<LotteryTicketId> ticket1 = service.submitTicket(LotteryTestUtils.createLotteryTicket("cvt@bbb.com",
        "123-12312", "+32425255", new HashSet<>(Arrays.asList(1, 2, 3, 4))));
    assertTrue(ticket1.isPresent());
    Optional<LotteryTicketId> ticket2 = service.submitTicket(LotteryTestUtils.createLotteryTicket("ant@bac.com",
        "123-12312", "+32423455", new HashSet<>(Arrays.asList(11, 12, 13, 14))));
    assertTrue(ticket2.isPresent());
    Optional<LotteryTicketId> ticket3 = service.submitTicket(LotteryTestUtils.createLotteryTicket("arg@boo.com",
        "123-12312", "+32421255", new HashSet<>(Arrays.asList(6, 8, 13, 19))));
    assertTrue(ticket3.isPresent());
    assertEquals(administration.getAllSubmittedTickets().size(), 3);
    
        LotteryNumbers winningNumbers = administration.performLottery();

        Optional<LotteryTicketId> ticket4 = service.submitTicket(LotteryTestUtils.createLotteryTicket("lucky@orb.com",
        "123-12312", "+12421255", winningNumbers.getNumbers()));
    assertTrue(ticket4.isPresent());
    assertEquals(administration.getAllSubmittedTickets().size(), 4);
    
        Map<LotteryTicketId, LotteryTicket> tickets = administration.getAllSubmittedTickets();
    for (LotteryTicketId id: tickets.keySet()) {
      LotteryTicketCheckResult checkResult = service.checkTicketForPrize(id, winningNumbers);
      assertTrue(checkResult.getResult() != CheckResult.TICKET_NOT_SUBMITTED);
      if (checkResult.getResult().equals(CheckResult.WIN_PRIZE)) {
        assertTrue(checkResult.getPrizeAmount() > 0);
      } else if (checkResult.getResult().equals(CheckResult.WIN_PRIZE)) {
        assertEquals(checkResult.getPrizeAmount(), 0);
      }
    }
    
        LotteryTicketCheckResult checkResult = service.checkTicketForPrize(new LotteryTicketId(), winningNumbers);
    assertTrue(checkResult.getResult() == CheckResult.TICKET_NOT_SUBMITTED);
    assertEquals(checkResult.getPrizeAmount(), 0);
  }
}
