

package com.google.common.util.concurrent;

import static java.util.logging.Level.SEVERE;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.annotations.VisibleForTesting;
import java.lang.Thread.UncaughtExceptionHandler;
import java.util.Locale;
import java.util.logging.Logger;


@GwtIncompatible
public final class UncaughtExceptionHandlers {
  private UncaughtExceptionHandlers() {}

  
  public static UncaughtExceptionHandler systemExit() {
    return new Exiter(Runtime.getRuntime());
  }

  @VisibleForTesting
  static final class Exiter implements UncaughtExceptionHandler {
    private static final Logger logger = Logger.getLogger(Exiter.class.getName());

    private final Runtime runtime;

    Exiter(Runtime runtime) {
      this.runtime = runtime;
    }

    @Override
    public void uncaughtException(Thread t, Throwable e) {
      try {
                logger.log(
            SEVERE, String.format(Locale.ROOT, "Caught an exception in %s.  Shutting down.", t), e);
      } catch (Throwable errorInLogging) {
                        System.err.println(e.getMessage());
        System.err.println(errorInLogging.getMessage());
      } finally {
        runtime.exit(1);
      }
    }
  }
}
