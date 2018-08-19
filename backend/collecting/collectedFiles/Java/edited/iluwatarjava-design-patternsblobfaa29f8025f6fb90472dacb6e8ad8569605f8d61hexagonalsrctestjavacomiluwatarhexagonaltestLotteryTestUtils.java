
package com.iluwatar.hexagonal.test;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import com.iluwatar.hexagonal.domain.LotteryNumbers;
import com.iluwatar.hexagonal.domain.LotteryTicket;
import com.iluwatar.hexagonal.domain.LotteryTicketId;
import com.iluwatar.hexagonal.domain.PlayerDetails;


public class LotteryTestUtils {

  
  public static LotteryTicket createLotteryTicket() {
    return createLotteryTicket("foo@bar.com", "12231-213132", "+99324554", new HashSet<>(Arrays.asList(1, 2, 3, 4)));
  }
  
  
  public static LotteryTicket createLotteryTicket(String email, String account, String phone,
      Set<Integer> givenNumbers) {
    PlayerDetails details = new PlayerDetails(email, account, phone);
    LotteryNumbers numbers = LotteryNumbers.create(givenNumbers);
    return new LotteryTicket(new LotteryTicketId(), details, numbers);
  }
}
