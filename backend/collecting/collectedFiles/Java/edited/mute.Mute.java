
package com.iluwatar.mute;

import java.io.ByteArrayOutputStream;
import java.io.IOException;


public final class Mute {
  
    private Mute() {}

  
  public static void mute(CheckedRunnable runnable) {
    try {
      runnable.run();
    } catch (Exception e) {
      throw new AssertionError(e);
    }
  }

  
  public static void loggedMute(CheckedRunnable runnable) {
    try {
      runnable.run();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
}
