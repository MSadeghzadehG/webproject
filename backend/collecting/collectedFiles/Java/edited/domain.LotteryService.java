
package com.iluwatar.hexagonal.domain;

import com.google.inject.Inject;
import com.iluwatar.hexagonal.banking.WireTransfers;
import com.iluwatar.hexagonal.database.LotteryTicketRepository;
import com.iluwatar.hexagonal.eventlog.LotteryEventLog;

import java.util.Optional;


public class LotteryService {

  private final LotteryTicketRepository repository;
  private final LotteryEventLog notifications;
  private final WireTransfers wireTransfers;

  
  @Inject
  public LotteryService(LotteryTicketRepository repository, LotteryEventLog notifications,
                        WireTransfers wireTransfers) {
    this.repository = repository;
    this.notifications = notifications;
    this.wireTransfers = wireTransfers;
  }

  
  public Optional<LotteryTicketId> submitTicket(LotteryTicket ticket) {
    boolean result = wireTransfers.transferFunds(LotteryConstants.TICKET_PRIZE,
        ticket.getPlayerDetails().getBankAccount(), LotteryConstants.SERVICE_BANK_ACCOUNT);
    if (!result) {
      notifications.ticketSubmitError(ticket.getPlayerDetails());
      return Optional.empty();
    }
    Optional<LotteryTicketId> optional = repository.save(ticket);
    if (optional.isPresent()) {
      notifications.ticketSubmitted(ticket.getPlayerDetails());
    }
    return optional;
  }

  
  public LotteryTicketCheckResult checkTicketForPrize(LotteryTicketId id, LotteryNumbers winningNumbers) {
    return LotteryUtils.checkTicketForPrize(repository, id, winningNumbers);
  }
}
