

package com.iluwatar.retry;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public final class App {
  private static final Logger LOG = LoggerFactory.getLogger(App.class);
  private static BusinessOperation<String> op;

  
  public static void main(String[] args) throws Exception {
    noErrors();
    errorNoRetry();
    errorWithRetry();
  }

  private static void noErrors() throws Exception {
    op = new FindCustomer("123");
    op.perform();
    LOG.info("Sometimes the operation executes with no errors.");
  }

  private static void errorNoRetry() throws Exception {
    op = new FindCustomer("123", new CustomerNotFoundException("not found"));
    try {
      op.perform();
    } catch (CustomerNotFoundException e) {
      LOG.info("Yet the operation will throw an error every once in a while.");
    }
  }

  private static void errorWithRetry() throws Exception {
    final Retry<String> retry = new Retry<>(
        new FindCustomer("123", new CustomerNotFoundException("not found")),
        3,          100,         e -> CustomerNotFoundException.class.isAssignableFrom(e.getClass())
    );
    op = retry;
    final String customerId = op.perform();
    LOG.info(String.format(
        "However, retrying the operation while ignoring a recoverable error will eventually yield "
        + "the result %s after a number of attempts %s", customerId, retry.attempts()
    ));
  }
}
