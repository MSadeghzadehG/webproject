
package com.iluwatar.resource.acquisition.is.initialization;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) throws Exception {

    try (SlidingDoor slidingDoor = new SlidingDoor()) {
      LOGGER.info("Walking in.");
    }

    try (TreasureChest treasureChest = new TreasureChest()) {
      LOGGER.info("Looting contents.");
    }
  }
}
