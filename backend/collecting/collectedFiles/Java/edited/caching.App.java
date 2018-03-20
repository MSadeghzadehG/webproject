
package com.iluwatar.caching;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);


  
  public static void main(String[] args) {
    AppManager.initDb(false);                                                                                               AppManager.initCacheCapacity(3);
    App app = new App();
    app.useReadAndWriteThroughStrategy();
    app.useReadThroughAndWriteAroundStrategy();
    app.useReadThroughAndWriteBehindStrategy();
    app.useCacheAsideStategy();
  }

  
  public void useReadAndWriteThroughStrategy() {
    LOGGER.info("# CachingPolicy.THROUGH");
    AppManager.initCachingPolicy(CachingPolicy.THROUGH);

    UserAccount userAccount1 = new UserAccount("001", "John", "He is a boy.");

    AppManager.save(userAccount1);
    LOGGER.info(AppManager.printCacheContent());
    AppManager.find("001");
    AppManager.find("001");
  }

  
  public void useReadThroughAndWriteAroundStrategy() {
    LOGGER.info("# CachingPolicy.AROUND");
    AppManager.initCachingPolicy(CachingPolicy.AROUND);

    UserAccount userAccount2 = new UserAccount("002", "Jane", "She is a girl.");

    AppManager.save(userAccount2);
    LOGGER.info(AppManager.printCacheContent());
    AppManager.find("002");
    LOGGER.info(AppManager.printCacheContent());
    userAccount2 = AppManager.find("002");
    userAccount2.setUserName("Jane G.");
    AppManager.save(userAccount2);
    LOGGER.info(AppManager.printCacheContent());
    AppManager.find("002");
    LOGGER.info(AppManager.printCacheContent());
    AppManager.find("002");
  }

  
  public void useReadThroughAndWriteBehindStrategy() {
    LOGGER.info("# CachingPolicy.BEHIND");
    AppManager.initCachingPolicy(CachingPolicy.BEHIND);

    UserAccount userAccount3 = new UserAccount("003", "Adam", "He likes food.");
    UserAccount userAccount4 = new UserAccount("004", "Rita", "She hates cats.");
    UserAccount userAccount5 = new UserAccount("005", "Isaac", "He is allergic to mustard.");

    AppManager.save(userAccount3);
    AppManager.save(userAccount4);
    AppManager.save(userAccount5);
    LOGGER.info(AppManager.printCacheContent());
    AppManager.find("003");
    LOGGER.info(AppManager.printCacheContent());
    UserAccount userAccount6 = new UserAccount("006", "Yasha", "She is an only child.");
    AppManager.save(userAccount6);
    LOGGER.info(AppManager.printCacheContent());
    AppManager.find("004");
    LOGGER.info(AppManager.printCacheContent());
  }

  
  public void useCacheAsideStategy() {
    LOGGER.info("# CachingPolicy.ASIDE");
    AppManager.initCachingPolicy(CachingPolicy.ASIDE);
    LOGGER.info(AppManager.printCacheContent());

    UserAccount userAccount3 = new UserAccount("003", "Adam", "He likes food.");
    UserAccount userAccount4 = new UserAccount("004", "Rita", "She hates cats.");
    UserAccount userAccount5 = new UserAccount("005", "Isaac", "He is allergic to mustard.");
    AppManager.save(userAccount3);
    AppManager.save(userAccount4);
    AppManager.save(userAccount5);

    LOGGER.info(AppManager.printCacheContent());
    AppManager.find("003");
    LOGGER.info(AppManager.printCacheContent());
    AppManager.find("004");
    LOGGER.info(AppManager.printCacheContent());
  }
}
