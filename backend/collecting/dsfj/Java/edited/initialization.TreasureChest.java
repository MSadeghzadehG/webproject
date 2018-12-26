
package com.iluwatar.resource.acquisition.is.initialization;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.Closeable;
import java.io.IOException;


public class TreasureChest implements Closeable {

  private static final Logger LOGGER = LoggerFactory.getLogger(TreasureChest.class);

  public TreasureChest() {
    LOGGER.info("Treasure chest opens.");
  }

  @Override
  public void close() throws IOException {
    LOGGER.info("Treasure chest closes.");
  }
}
