

package com.google.common.math;

import com.google.common.annotations.GwtCompatible;


@GwtCompatible(emulated = true)
class TestPlatform {
  static boolean intsCanGoOutOfRange() {
    return false;
  }

  static boolean isAndroid() {
    return System.getProperties().getProperty("java.runtime.name").contains("Android");
  }
}
