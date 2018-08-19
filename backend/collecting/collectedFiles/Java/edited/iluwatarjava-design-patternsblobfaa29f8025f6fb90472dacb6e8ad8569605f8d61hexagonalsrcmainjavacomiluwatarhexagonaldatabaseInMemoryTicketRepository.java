
package com.iluwatar.hexagonal.database;

import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

import com.iluwatar.hexagonal.domain.LotteryTicket;
import com.iluwatar.hexagonal.domain.LotteryTicketId;


public class InMemoryTicketRepository implements LotteryTicketRepository {
  
  private static Map<LotteryTicketId, LotteryTicket> tickets = new HashMap<>();

  @Override
  public Optional<LotteryTicket> findById(LotteryTicketId id) {
    LotteryTicket ticket = tickets.get(id);
    if (ticket == null) {
      return Optional.empty();
    } else {
      return Optional.of(ticket);
    }
  }

  @Override
  public Optional<LotteryTicketId> save(LotteryTicket ticket) {
    LotteryTicketId id = new LotteryTicketId();
    tickets.put(id, ticket);
    return Optional.of(id);
  }

  @Override
  public Map<LotteryTicketId, LotteryTicket> findAll() {
    return tickets;
  }

  @Override
  public void deleteAll() {
    tickets.clear();
  }
}
