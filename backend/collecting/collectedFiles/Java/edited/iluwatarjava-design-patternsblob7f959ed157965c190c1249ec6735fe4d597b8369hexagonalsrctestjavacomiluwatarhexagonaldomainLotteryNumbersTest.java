
package com.iluwatar.hexagonal.domain;

import org.junit.jupiter.api.Test;

import java.util.Arrays;
import java.util.HashSet;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;


public class LotteryNumbersTest {
  
  @Test
  public void testGivenNumbers() {
    LotteryNumbers numbers = LotteryNumbers.create(
            new HashSet<>(Arrays.asList(1, 2, 3, 4)));
    assertEquals(numbers.getNumbers().size(), 4);
    assertTrue(numbers.getNumbers().contains(1));
    assertTrue(numbers.getNumbers().contains(2));
    assertTrue(numbers.getNumbers().contains(3));
    assertTrue(numbers.getNumbers().contains(4));
  }
  
  @Test
  public void testNumbersCantBeModified() {
    LotteryNumbers numbers = LotteryNumbers.create(
            new HashSet<>(Arrays.asList(1, 2, 3, 4)));
    assertThrows(UnsupportedOperationException.class, () -> {
      numbers.getNumbers().add(5);
    });
  }
  
  @Test
  public void testRandomNumbers() {
    LotteryNumbers numbers = LotteryNumbers.createRandom();
    assertEquals(numbers.getNumbers().size(), LotteryNumbers.NUM_NUMBERS);
  }
  
  @Test
  public void testEquals() {
    LotteryNumbers numbers1 = LotteryNumbers.create(
            new HashSet<>(Arrays.asList(1, 2, 3, 4)));
    LotteryNumbers numbers2 = LotteryNumbers.create(
            new HashSet<>(Arrays.asList(1, 2, 3, 4)));
    assertTrue(numbers1.equals(numbers2));
    LotteryNumbers numbers3 = LotteryNumbers.create(
            new HashSet<>(Arrays.asList(11, 12, 13, 14)));
    assertFalse(numbers1.equals(numbers3));
  }
}
