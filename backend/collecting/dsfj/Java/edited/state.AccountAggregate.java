
package com.iluwatar.event.sourcing.state;

import com.iluwatar.event.sourcing.domain.Account;
import java.util.HashMap;
import java.util.Map;


public class AccountAggregate {

  private static Map<Integer, Account> accounts = new HashMap<>();

  private AccountAggregate() {
  }

  
  public static void putAccount(Account account) {
    accounts.put(account.getAccountNo(), account);
  }

  
  public static Account getAccount(int accountNo) {
    Account account = accounts.get(accountNo);
    if (account == null) {
      return null;
    }
    return account.copy();
  }

  
  public static void resetState() {
    accounts = new HashMap<>();
  }
}
