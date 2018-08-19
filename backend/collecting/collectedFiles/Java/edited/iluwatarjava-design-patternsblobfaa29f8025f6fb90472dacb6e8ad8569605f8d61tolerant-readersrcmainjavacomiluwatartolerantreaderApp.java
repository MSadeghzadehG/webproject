
package com.iluwatar.tolerantreader;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) throws IOException, ClassNotFoundException {
        RainbowFish fishV1 = new RainbowFish("Zed", 10, 11, 12);
    LOGGER.info("fishV1 name={} age={} length={} weight={}", fishV1.getName(),
        fishV1.getAge(), fishV1.getLengthMeters(), fishV1.getWeightTons());
    RainbowFishSerializer.writeV1(fishV1, "fish1.out");
        RainbowFish deserializedFishV1 = RainbowFishSerializer.readV1("fish1.out");
    LOGGER.info("deserializedFishV1 name={} age={} length={} weight={}",
        deserializedFishV1.getName(), deserializedFishV1.getAge(),
        deserializedFishV1.getLengthMeters(), deserializedFishV1.getWeightTons());
        RainbowFishV2 fishV2 = new RainbowFishV2("Scar", 5, 12, 15, true, true, true);
    LOGGER.info(
        "fishV2 name={} age={} length={} weight={} sleeping={} hungry={} angry={}",
        fishV2.getName(), fishV2.getAge(), fishV2.getLengthMeters(), fishV2.getWeightTons(),
        fishV2.getHungry(), fishV2.getAngry(), fishV2.getSleeping());
    RainbowFishSerializer.writeV2(fishV2, "fish2.out");
        RainbowFish deserializedFishV2 = RainbowFishSerializer.readV1("fish2.out");
    LOGGER.info("deserializedFishV2 name={} age={} length={} weight={}",
        deserializedFishV2.getName(), deserializedFishV2.getAge(),
        deserializedFishV2.getLengthMeters(), deserializedFishV2.getWeightTons());
  }
}
