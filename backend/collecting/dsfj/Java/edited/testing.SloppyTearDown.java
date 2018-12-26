

package com.google.common.testing;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import java.util.logging.Level;
import java.util.logging.Logger;


@Beta
@GwtCompatible
public abstract class SloppyTearDown implements TearDown {
  private static final Logger logger = Logger.getLogger(SloppyTearDown.class.getName());

  @Override
  public final void tearDown() {
    try {
      sloppyTearDown();
    } catch (Throwable t) {
      logger.log(Level.INFO, "exception thrown during tearDown: " + t.getMessage(), t);
    }
  }

  public abstract void sloppyTearDown() throws Exception;
}
