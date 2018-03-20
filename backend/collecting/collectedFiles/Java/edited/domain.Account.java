
package com.iluwatar.event.sourcing.domain;

import com.iluwatar.event.sourcing.event.AccountCreateEvent;
import com.iluwatar.event.sourcing.event.MoneyDepositEvent;
import com.iluwatar.event.sourcing.event.MoneyTransferEvent;
import com.iluwatar.event.sourcing.state.AccountAggregate;
import java.math.BigDecimal;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class Account {

  private static final Logger LOGGER = LoggerFactory.getLogger(Account.class);

  private final int accountNo;
  private final String owner;
  private BigDecimal money;

  
  public Account(int accountNo, String owner) {
    this.accountNo = accountNo;
    this.owner = owner;
    money = BigDecimal.ZERO;
  }

  
  public int getAccountNo() {
    return accountNo;
  }

  
  public String getOwner() {
    return owner;
  }

  
  public BigDecimal getMoney() {
    return money;
  }

  
  public void setMoney(BigDecimal money) {
    this.money = money;
  }


  
  public Account copy() {
    Account account = new Account(accountNo, owner);
    account.setMoney(money);
    return account;
  }

  @Override
  public String toString() {
    return "Account{"
        + "accountNo=" + accountNo
        + ", owner='" + owner + '\''
        + ", money=" + money
        + '}';
  }

  private void depositMoney(BigDecimal money) {
    this.money = this.money.add(money);
  }

  private void withdrawMoney(BigDecimal money) {
    this.money = this.money.subtract(money);
  }

  private void handleDeposit(BigDecimal money, boolean realTime) {
    depositMoney(money);
    AccountAggregate.putAccount(this);
    if (realTime) {
      LOGGER.info("Some external api for only realtime execution could be called here.");
    }
  }

  private void handleWithdrawal(BigDecimal money, boolean realTime) {
    if (this.money.compareTo(money) == -1) {
      throw new RuntimeException("Insufficient Account Balance");
    }

    withdrawMoney(money);
    AccountAggregate.putAccount(this);
    if (realTime) {
      LOGGER.info("Some external api for only realtime execution could be called here.");
    }
  }

  
  public void handleEvent(MoneyDepositEvent moneyDepositEvent) {
    handleDeposit(moneyDepositEvent.getMoney(), moneyDepositEvent.isRealTime());
  }


  
  public void handleEvent(AccountCreateEvent accountCreateEvent) {
    AccountAggregate.putAccount(this);
    if (accountCreateEvent.isRealTime()) {
      LOGGER.info("Some external api for only realtime execution could be called here.");
    }
  }

  
  public void handleTransferFromEvent(MoneyTransferEvent moneyTransferEvent) {
    handleWithdrawal(moneyTransferEvent.getMoney(), moneyTransferEvent.isRealTime());
  }

  
  public void handleTransferToEvent(MoneyTransferEvent moneyTransferEvent) {
    handleDeposit(moneyTransferEvent.getMoney(), moneyTransferEvent.isRealTime());
  }


}
