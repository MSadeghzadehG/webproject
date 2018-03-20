
package com.iluwatar.lazy.loading;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.function.Supplier;


public class Java8Holder {

  private static final Logger LOGGER = LoggerFactory.getLogger(Java8Holder.class);

  private Supplier<Heavy> heavy = () -> createAndCacheHeavy();

  public Java8Holder() {
    LOGGER.info("Java8Holder created");
  }

  public Heavy getHeavy() {
    return heavy.get();
  }

  private synchronized Heavy createAndCacheHeavy() {
    class HeavyFactory implements Supplier<Heavy> {
      private final Heavy heavyInstance = new Heavy();

      @Override
      public Heavy get() {
        return heavyInstance;
      }
    }
    if (!HeavyFactory.class.isInstance(heavy)) {
      heavy = new HeavyFactory();
    }
    return heavy.get();
  }
}
