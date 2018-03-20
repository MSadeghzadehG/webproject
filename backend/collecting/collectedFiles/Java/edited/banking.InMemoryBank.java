
package com.iluwatar.hexagonal.banking;

import java.util.HashMap;
import java.util.Map;

import com.iluwatar.hexagonal.domain.LotteryConstants;


public class InMemoryBank implements WireTransfers {

  private static Map<String, Integer> accounts = new HashMap<>();
  
  static {
    accounts.put(LotteryConstants.SERVICE_BANK_ACCOUNT, LotteryConstants.SERVICE_BANK_ACCOUNT_SALDO);
  }
  
  @Override
  public void setFunds(String bankAccount, int amount) {
    accounts.put(bankAccount, amount);
  }

  @Override
  public int getFunds(String bankAccount) {
    return accounts.getOrDefault(bankAccount, 0);
  }

  @Override
  public boolean transferFunds(int amount, String sourceBackAccount, String destinationBankAccount) {
    if (accounts.getOrDefault(sourceBackAccount, 0) >= amount) {
      accounts.put(sourceBackAccount, accounts.get(sourceBackAccount) - amount);
      accounts.put(destinationBankAccount, accounts.get(destinationBankAccount) + amount);
      return true;
    } else {
      return false;
    }
  }
}
