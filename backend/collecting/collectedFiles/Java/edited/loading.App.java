
package com.iluwatar.lazy.loading;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) {

        HolderNaive holderNaive = new HolderNaive();
    Heavy heavy = holderNaive.getHeavy();
    LOGGER.info("heavy={}", heavy);

        HolderThreadSafe holderThreadSafe = new HolderThreadSafe();
    Heavy another = holderThreadSafe.getHeavy();
    LOGGER.info("another={}", another);

        Java8Holder java8Holder = new Java8Holder();
    Heavy next = java8Holder.getHeavy();
    LOGGER.info("next={}", next);
  }
}
