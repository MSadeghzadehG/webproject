
package com.iluwatar.event.aggregator;

import java.util.ArrayList;
import java.util.List;


public class App {

  
  public static void main(String[] args) {

    KingJoffrey kingJoffrey = new KingJoffrey();
    KingsHand kingsHand = new KingsHand(kingJoffrey);

    List<EventEmitter> emitters = new ArrayList<>();
    emitters.add(kingsHand);
    emitters.add(new LordBaelish(kingsHand));
    emitters.add(new LordVarys(kingsHand));
    emitters.add(new Scout(kingsHand));

    for (Weekday day : Weekday.values()) {
      for (EventEmitter emitter : emitters) {
        emitter.timePasses(day);
      }
    }
  }
}
