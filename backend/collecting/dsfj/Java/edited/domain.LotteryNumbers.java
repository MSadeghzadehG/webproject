
package com.iluwatar.hexagonal.domain;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.PrimitiveIterator;
import java.util.Random;
import java.util.Set;


public class LotteryNumbers {

  private final Set<Integer> numbers;
  
  public static final int MIN_NUMBER = 1;
  public static final int MAX_NUMBER = 20;
  public static final int NUM_NUMBERS = 4;

  
  private LotteryNumbers() {
    numbers = new HashSet<>();
    generateRandomNumbers();
  }

  
  private LotteryNumbers(Set<Integer> givenNumbers) {
    numbers = new HashSet<>();
    numbers.addAll(givenNumbers);
  }

  
  public static LotteryNumbers createRandom() {
    return new LotteryNumbers();
  }

  
  public static LotteryNumbers create(Set<Integer> givenNumbers) {
    return new LotteryNumbers(givenNumbers);
  }
  
  
  public Set<Integer> getNumbers() {
    return Collections.unmodifiableSet(numbers);
  }

  
  public String getNumbersAsString() {
    List<Integer> list = new ArrayList<>();
    list.addAll(numbers);
    StringBuilder builder = new StringBuilder();
    for (int i = 0; i < NUM_NUMBERS; i++) {
      builder.append(list.get(i));
      if (i < NUM_NUMBERS - 1) {
        builder.append(",");
      }
    }
    return builder.toString();
  }
  
  
  private void generateRandomNumbers() {
    numbers.clear();
    RandomNumberGenerator generator = new RandomNumberGenerator(MIN_NUMBER, MAX_NUMBER);
    while (numbers.size() < NUM_NUMBERS) {
      int num = generator.nextInt();
      if (!numbers.contains(num)) {
        numbers.add(num);
      }
    }
  }

  @Override
  public String toString() {
    return "LotteryNumbers{" + "numbers=" + numbers + '}';
  }

  
  private static class RandomNumberGenerator {

    private PrimitiveIterator.OfInt randomIterator;

    
    public RandomNumberGenerator(int min, int max) {
      randomIterator = new Random().ints(min, max + 1).iterator();
    }

    
    public int nextInt() {
      return randomIterator.nextInt();
    }
  }
  
  @Override
  public int hashCode() {
    final int prime = 31;
    int result = 1;
    result = prime * result + ((numbers == null) ? 0 : numbers.hashCode());
    return result;
  }

  @Override
  public boolean equals(Object obj) {
    if (this == obj) {
      return true;
    }
    if (obj == null) {
      return false;
    }
    if (getClass() != obj.getClass()) {
      return false;
    }
    LotteryNumbers other = (LotteryNumbers) obj;
    if (numbers == null) {
      if (other.numbers != null) {
        return false;
      }
    } else if (!numbers.equals(other.numbers)) {
      return false;
    }
    return true;
  }  
}
