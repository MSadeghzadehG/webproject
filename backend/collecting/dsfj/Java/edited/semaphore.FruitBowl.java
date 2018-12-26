
package com.iluwatar.semaphore;

import java.util.List;
import java.util.ArrayList;


public class FruitBowl {
    
  private List<Fruit> fruit = new ArrayList<>();

  
  public int countFruit() {
    return fruit.size();
  }

  
  public void put(Fruit f) {
    fruit.add(f);
  }
  
  
  public Fruit take() {
    if (fruit.isEmpty()) {
      return null;
    } else {
      return fruit.remove(0);
    }
  }
  
     
  public String toString() {
    int apples = 0;
    int oranges = 0;
    int lemons = 0;
        
    for (Fruit f : fruit) {
      switch (f.getType()) {
        case APPLE:
          apples++;
          break;
        case ORANGE:
          oranges++;
          break;
        case LEMON:
          lemons++;
          break;
        default:
      }
    }
        
    return apples + " Apples, " + oranges + " Oranges, and " + lemons + " Lemons";
  }
}
