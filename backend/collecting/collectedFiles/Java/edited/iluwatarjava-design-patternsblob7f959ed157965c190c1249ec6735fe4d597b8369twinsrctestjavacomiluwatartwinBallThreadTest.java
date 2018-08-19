
package com.iluwatar.twin;

import org.junit.jupiter.api.Test;

import static java.lang.Thread.UncaughtExceptionHandler;
import static java.lang.Thread.sleep;
import static java.time.Duration.ofMillis;
import static org.junit.jupiter.api.Assertions.assertTimeout;
import static org.mockito.Matchers.any;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;


public class BallThreadTest {

  
  @Test
  public void testSuspend() throws Exception {
    assertTimeout(ofMillis(5000), () -> {
      final BallThread ballThread = new BallThread();

      final BallItem ballItem = mock(BallItem.class);
      ballThread.setTwin(ballItem);

      ballThread.start();

      verify(ballItem, timeout(2000).atLeastOnce()).draw();
      verify(ballItem, timeout(2000).atLeastOnce()).move();
      ballThread.suspendMe();

      sleep(1000);

      ballThread.stopMe();
      ballThread.join();

      verifyNoMoreInteractions(ballItem);
    });
  }

  
  @Test
  public void testResume() throws Exception {
    assertTimeout(ofMillis(5000), () -> {
      final BallThread ballThread = new BallThread();

      final BallItem ballItem = mock(BallItem.class);
      ballThread.setTwin(ballItem);

      ballThread.suspendMe();
      ballThread.start();

      sleep(1000);

      verifyZeroInteractions(ballItem);

      ballThread.resumeMe();
      verify(ballItem, timeout(2000).atLeastOnce()).draw();
      verify(ballItem, timeout(2000).atLeastOnce()).move();

      ballThread.stopMe();
      ballThread.join();

      verifyNoMoreInteractions(ballItem);
    });
  }

  
  @Test
  public void testInterrupt() throws Exception {
    assertTimeout(ofMillis(5000), () -> {
      final BallThread ballThread = new BallThread();
      final UncaughtExceptionHandler exceptionHandler = mock(UncaughtExceptionHandler.class);
      ballThread.setUncaughtExceptionHandler(exceptionHandler);
      ballThread.setTwin(mock(BallItem.class));
      ballThread.start();
      ballThread.interrupt();
      ballThread.join();

      verify(exceptionHandler).uncaughtException(eq(ballThread), any(RuntimeException.class));
      verifyNoMoreInteractions(exceptionHandler);
    });
  }
}