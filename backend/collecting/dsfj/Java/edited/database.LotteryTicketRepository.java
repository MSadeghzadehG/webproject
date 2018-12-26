
package com.iluwatar.hexagonal.database;

import java.util.Map;
import java.util.Optional;

import com.iluwatar.hexagonal.domain.LotteryTicket;
import com.iluwatar.hexagonal.domain.LotteryTicketId;


public interface LotteryTicketRepository {

  
  Optional<LotteryTicket> findById(LotteryTicketId id);

  
  Optional<LotteryTicketId> save(LotteryTicket ticket);

  
  Map<LotteryTicketId, LotteryTicket> findAll();

  
  void deleteAll();
  
}
