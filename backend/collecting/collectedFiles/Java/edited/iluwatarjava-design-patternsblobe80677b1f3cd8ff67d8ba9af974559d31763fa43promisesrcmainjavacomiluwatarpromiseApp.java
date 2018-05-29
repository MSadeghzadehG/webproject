
package com.iluwatar.promise;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  private static final String DEFAULT_URL = "https:  private final ExecutorService executor;
  private final CountDownLatch stopLatch;

  private App() {
    executor = Executors.newFixedThreadPool(2);
    stopLatch = new CountDownLatch(2);
  }

  
  public static void main(String[] args) throws InterruptedException, ExecutionException {
    App app = new App();
    try {
      app.promiseUsage();
    } finally {
      app.stop();
    }
  }

  private void promiseUsage() {
    calculateLineCount();

    calculateLowestFrequencyChar();
  }

  
  private void calculateLowestFrequencyChar() {
    lowestFrequencyChar()
        .thenAccept(
          charFrequency -> {
            LOGGER.info("Char with lowest frequency is: {}", charFrequency);
            taskCompleted();
          }
      );
  }

  
  private void calculateLineCount() {
    countLines()
        .thenAccept(
          count -> {
            LOGGER.info("Line count is: {}", count);
            taskCompleted();
          }
      );
  }

  
  private Promise<Character> lowestFrequencyChar() {
    return characterFrequency()
        .thenApply(Utility::lowestFrequencyChar);
  }

  
  private Promise<Map<Character, Integer>> characterFrequency() {
    return download(DEFAULT_URL)
        .thenApply(Utility::characterFrequency);
  }

  
  private Promise<Integer> countLines() {
    return download(DEFAULT_URL)
        .thenApply(Utility::countLines);
  }

  
  private Promise<String> download(String urlString) {
    Promise<String> downloadPromise = new Promise<String>()
        .fulfillInAsync(
            () -> {
              return Utility.downloadFile(urlString);
            }, executor)
        .onError(
            throwable -> {
              throwable.printStackTrace();
              taskCompleted();
            }
        );

    return downloadPromise;
  }

  private void stop() throws InterruptedException {
    stopLatch.await();
    executor.shutdownNow();
  }

  private void taskCompleted() {
    stopLatch.countDown();
  }
}
