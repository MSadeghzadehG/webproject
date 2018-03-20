

package com.google.zxing.web;

import java.util.Iterator;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.atomic.AtomicLong;
import java.util.logging.Logger;


final class DoSTracker {

  private static final Logger log = Logger.getLogger(DoSTracker.class.getName());

  private final long maxAccessesPerTime;
  private final Map<String,AtomicLong> numRecentAccesses;

  DoSTracker(Timer timer, final int maxAccessesPerTime, long accessTimeMS, int maxEntries) {
    this.maxAccessesPerTime = maxAccessesPerTime;
    this.numRecentAccesses = new LRUMap<>(maxEntries);
    timer.scheduleAtFixedRate(new TimerTask() {
      @Override
      public void run() {
        synchronized (numRecentAccesses) {
          Iterator<Map.Entry<String,AtomicLong>> accessIt = numRecentAccesses.entrySet().iterator();
          while (accessIt.hasNext()) {
            Map.Entry<String,AtomicLong> entry = accessIt.next();
            AtomicLong count = entry.getValue();
                        if (count.get() <= maxAccessesPerTime) {
              accessIt.remove();
            } else {
                            log.warning("Blocking " + entry.getKey() + " (" + count + " outstanding)");
                            count.getAndAdd(-maxAccessesPerTime);
            }
          }
        }
      }
    }, accessTimeMS, accessTimeMS);

  }

  boolean isBanned(String event) {
    if (event == null) {
      return true;
    }
    AtomicLong count;
    synchronized (numRecentAccesses) {
      count = numRecentAccesses.get(event);
      if (count == null) {
        count = new AtomicLong();
        numRecentAccesses.put(event, count);
      }
    }
    return count.incrementAndGet() > maxAccessesPerTime;
  }

}
