
package com.iluwatar.caching;

import java.text.ParseException;


public final class AppManager {

  private static CachingPolicy cachingPolicy;

  private AppManager() {
  }

  
  public static void initDb(boolean useMongoDb) {
    if (useMongoDb) {
      try {
        DbManager.connect();
      } catch (ParseException e) {
        e.printStackTrace();
      }
    } else {
      DbManager.createVirtualDb();
    }
  }

  
  public static void initCachingPolicy(CachingPolicy policy) {
    cachingPolicy = policy;
    if (cachingPolicy == CachingPolicy.BEHIND) {
      Runtime.getRuntime().addShutdownHook(new Thread(CacheStore::flushCache));
    }
    CacheStore.clearCache();
  }

  public static void initCacheCapacity(int capacity) {
    CacheStore.initCapacity(capacity);
  }

  
  public static UserAccount find(String userId) {
    if (cachingPolicy == CachingPolicy.THROUGH || cachingPolicy == CachingPolicy.AROUND) {
      return CacheStore.readThrough(userId);
    } else if (cachingPolicy == CachingPolicy.BEHIND) {
      return CacheStore.readThroughWithWriteBackPolicy(userId);
    } else if (cachingPolicy == CachingPolicy.ASIDE) {
      return findAside(userId);
    }
    return null;
  }

  
  public static void save(UserAccount userAccount) {
    if (cachingPolicy == CachingPolicy.THROUGH) {
      CacheStore.writeThrough(userAccount);
    } else if (cachingPolicy == CachingPolicy.AROUND) {
      CacheStore.writeAround(userAccount);
    } else if (cachingPolicy == CachingPolicy.BEHIND) {
      CacheStore.writeBehind(userAccount);
    } else if (cachingPolicy == CachingPolicy.ASIDE) {
      saveAside(userAccount);
    }
  }

  public static String printCacheContent() {
    return CacheStore.print();
  }

  
  private static void saveAside(UserAccount userAccount) {
    DbManager.updateDb(userAccount);
    CacheStore.invalidate(userAccount.getUserId());
  }

  
  private static UserAccount findAside(String userId) {
    UserAccount userAccount = CacheStore.get(userId);
    if (userAccount != null) {
      return userAccount;
    }

    userAccount = DbManager.readFromDb(userId);
    if (userAccount != null) {
      CacheStore.set(userId, userAccount);
    }

    return userAccount;
  }
}
