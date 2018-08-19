
package com.iluwatar.hexagonal.database;

import com.iluwatar.hexagonal.domain.LotteryNumbers;
import com.iluwatar.hexagonal.domain.LotteryTicket;
import com.iluwatar.hexagonal.domain.LotteryTicketId;
import com.iluwatar.hexagonal.domain.PlayerDetails;
import com.iluwatar.hexagonal.mongo.MongoConnectionPropertiesLoader;
import com.mongodb.MongoClient;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;

import java.util.Optional;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;


@Disabled
public class MongoTicketRepositoryTest {

  private static final String TEST_DB = "lotteryTestDB";
  private static final String TEST_TICKETS_COLLECTION = "lotteryTestTickets";
  private static final String TEST_COUNTERS_COLLECTION = "testCounters";

  private MongoTicketRepository repository;

  @BeforeEach
  public void init() {
    MongoConnectionPropertiesLoader.load();
    MongoClient mongoClient = new MongoClient(System.getProperty("mongo-host"),
        Integer.parseInt(System.getProperty("mongo-port")));
    mongoClient.dropDatabase(TEST_DB);
    mongoClient.close();
    repository = new MongoTicketRepository(TEST_DB, TEST_TICKETS_COLLECTION,
        TEST_COUNTERS_COLLECTION);
  }

  @Test
  public void testSetup() {
    assertTrue(repository.getCountersCollection().count() == 1);
    assertTrue(repository.getTicketsCollection().count() == 0);
  }

  @Test
  public void testNextId() {
    assertEquals(1, repository.getNextId());
    assertEquals(2, repository.getNextId());
    assertEquals(3, repository.getNextId());
  }

  @Test
  public void testCrudOperations() {
        PlayerDetails details = new PlayerDetails("foo@bar.com", "123-123", "07001234");
    LotteryNumbers random = LotteryNumbers.createRandom();
    LotteryTicket original = new LotteryTicket(new LotteryTicketId(), details, random);
    Optional<LotteryTicketId> saved = repository.save(original);
    assertEquals(1, repository.getTicketsCollection().count());
    assertTrue(saved.isPresent());
        Optional<LotteryTicket> found = repository.findById(saved.get());
    assertTrue(found.isPresent());
    LotteryTicket ticket = found.get();
    assertEquals("foo@bar.com", ticket.getPlayerDetails().getEmail());
    assertEquals("123-123", ticket.getPlayerDetails().getBankAccount());
    assertEquals("07001234", ticket.getPlayerDetails().getPhoneNumber());
    assertEquals(original.getNumbers(), ticket.getNumbers());
        repository.deleteAll();
    assertEquals(0, repository.getTicketsCollection().count());
  }
}
