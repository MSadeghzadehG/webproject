

import com.iluwatar.event.sourcing.domain.Account;
import com.iluwatar.event.sourcing.event.AccountCreateEvent;
import com.iluwatar.event.sourcing.event.MoneyDepositEvent;
import com.iluwatar.event.sourcing.event.MoneyTransferEvent;
import com.iluwatar.event.sourcing.processor.DomainEventProcessor;
import com.iluwatar.event.sourcing.state.AccountAggregate;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.math.BigDecimal;
import java.util.Date;

import static com.iluwatar.event.sourcing.app.App.ACCOUNT_OF_DAENERYS;
import static com.iluwatar.event.sourcing.app.App.ACCOUNT_OF_JON;
import static org.junit.jupiter.api.Assertions.assertEquals;


public class IntegrationTest {

  
  private DomainEventProcessor eventProcessor;

  
  @BeforeEach
  public void initialize() {
    eventProcessor = new DomainEventProcessor();
  }

  
  @Test
  public void testStateRecovery() {
    eventProcessor.reset();

    eventProcessor.process(new AccountCreateEvent(
        0, new Date().getTime(), ACCOUNT_OF_DAENERYS, "Daenerys Targaryen"));

    eventProcessor.process(new AccountCreateEvent(
        1, new Date().getTime(), ACCOUNT_OF_JON, "Jon Snow"));

    eventProcessor.process(new MoneyDepositEvent(
        2, new Date().getTime(), ACCOUNT_OF_DAENERYS,  new BigDecimal("100000")));

    eventProcessor.process(new MoneyDepositEvent(
        3, new Date().getTime(), ACCOUNT_OF_JON,  new BigDecimal("100")));

    eventProcessor.process(new MoneyTransferEvent(
        4, new Date().getTime(), new BigDecimal("10000"), ACCOUNT_OF_DAENERYS,
        ACCOUNT_OF_JON));

    Account accountOfDaenerysBeforeShotDown = AccountAggregate.getAccount(ACCOUNT_OF_DAENERYS);
    Account accountOfJonBeforeShotDown = AccountAggregate.getAccount(ACCOUNT_OF_JON);

    AccountAggregate.resetState();

    eventProcessor = new DomainEventProcessor();
    eventProcessor.recover();

    Account accountOfDaenerysAfterShotDown = AccountAggregate.getAccount(ACCOUNT_OF_DAENERYS);
    Account accountOfJonAfterShotDown = AccountAggregate.getAccount(ACCOUNT_OF_JON);

    assertEquals(accountOfDaenerysBeforeShotDown.getMoney(),
        accountOfDaenerysAfterShotDown.getMoney());
    assertEquals(accountOfJonBeforeShotDown.getMoney(), accountOfJonAfterShotDown.getMoney());
  }

}