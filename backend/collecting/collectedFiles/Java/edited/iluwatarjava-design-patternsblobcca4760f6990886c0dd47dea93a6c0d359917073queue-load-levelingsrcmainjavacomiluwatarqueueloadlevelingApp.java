

package com.iluwatar.queue.load.leveling;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class App {
  
  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);
  
    private static final int SHUTDOWN_TIME = 15;
  
  
  public static void main(String[] args) {
    
            ExecutorService executor = null;
    
    try {
            MessageQueue msgQueue = new MessageQueue();
        
      LOGGER.info("Submitting TaskGenerators and ServiceExecutor threads.");
      
            Runnable taskRunnable1 = new TaskGenerator(msgQueue, 5);
      Runnable taskRunnable2 = new TaskGenerator(msgQueue, 1);
      Runnable taskRunnable3 = new TaskGenerator(msgQueue, 2);
      
            Runnable srvRunnable = new ServiceExecutor(msgQueue);

                  executor = Executors.newFixedThreadPool(2);
      executor.submit(taskRunnable1);
      executor.submit(taskRunnable2);
      executor.submit(taskRunnable3);
      
            executor.submit(srvRunnable);
      
            LOGGER.info("Intiating shutdown. Executor will shutdown only after all the Threads are completed.");
      executor.shutdown();
      
                  if ( !executor.awaitTermination(SHUTDOWN_TIME, TimeUnit.SECONDS) ) {
        LOGGER.info("Executor was shut down and Exiting.");
        executor.shutdownNow();
      }
    } catch (InterruptedException ie) {
      LOGGER.error(ie.getMessage());
    } catch (Exception e) {
      LOGGER.error(e.getMessage());
    }
  }
}