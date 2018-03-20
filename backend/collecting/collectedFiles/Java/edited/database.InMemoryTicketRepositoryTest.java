
package com.iluwatar.hexagonal.database;

import java.util.Optional;

import com.iluwatar.hexagonal.domain.LotteryTicket;
import com.iluwatar.hexagonal.domain.LotteryTicketId;
import com.iluwatar.hexagonal.test.LotteryTestUtils;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;


public class InMemoryTicketRepositoryTest {

  private final LotteryTicketRepository repository = new InMemoryTicketRepository();
  
  @BeforeEach
  public void clear() {
    repository.deleteAll();
  }
  
  @Test
  public void testCrudOperations() {
    LotteryTicketRepository repository = new InMemoryTicketRepository();
    assertEquals(repository.findAll().size(), 0);
    LotteryTicket ticket = LotteryTestUtils.createLotteryTicket();
    Optional<LotteryTicketId> id = repository.save(ticket);
    assertTrue(id.isPresent());
    assertEquals(repository.findAll().size(), 1);
    Optional<LotteryTicket> optionalTicket = repository.findById(id.get());
    assertTrue(optionalTicket.isPresent());
  }
}
