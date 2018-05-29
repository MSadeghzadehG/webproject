
package com.iluwatar.servant;

import java.util.List;


public class Servant {

  public String name;

  
  public Servant(String name) {
    this.name = name;
  }

  public void feed(Royalty r) {
    r.getFed();
  }

  public void giveWine(Royalty r) {
    r.getDrink();
  }

  public void giveCompliments(Royalty r) {
    r.receiveCompliments();
  }

  
  public boolean checkIfYouWillBeHanged(List<Royalty> tableGuests) {
    boolean anotherDay = true;
    for (Royalty r : tableGuests) {
      if (!r.getMood()) {
        anotherDay = false;
      }
    }

    return anotherDay;
  }
}
