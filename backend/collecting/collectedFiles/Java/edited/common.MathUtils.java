


package org.jbox2d.common;

import java.util.Random;


public class MathUtils extends PlatformMathUtils {
  public static final float PI = (float) Math.PI;
  public static final float TWOPI = (float) (Math.PI * 2);
  public static final float INV_PI = 1f / PI;
  public static final float HALF_PI = PI / 2;
  public static final float QUARTER_PI = PI / 4;
  public static final float THREE_HALVES_PI = TWOPI - HALF_PI;

  
  public static final float DEG2RAD = PI / 180;

  
  public static final float RAD2DEG = 180 / PI;

  public static final float[] sinLUT = new float[Settings.SINCOS_LUT_LENGTH];

  static {
    for (int i = 0; i < Settings.SINCOS_LUT_LENGTH; i++) {
      sinLUT[i] = (float) Math.sin(i * Settings.SINCOS_LUT_PRECISION);
    }
  }

  public static final float sin(float x) {
    if (Settings.SINCOS_LUT_ENABLED) {
      return sinLUT(x);
    } else {
      return (float) StrictMath.sin(x);
    }
  }

  public static final float sinLUT(float x) {
    x %= TWOPI;

    if (x < 0) {
      x += TWOPI;
    }

    if (Settings.SINCOS_LUT_LERP) {

      x /= Settings.SINCOS_LUT_PRECISION;

      final int index = (int) x;

      if (index != 0) {
        x %= index;
      }

            if (index == Settings.SINCOS_LUT_LENGTH - 1) {
        return ((1 - x) * sinLUT[index] + x * sinLUT[0]);
      } else {
        return ((1 - x) * sinLUT[index] + x * sinLUT[index + 1]);
      }

    } else {
      return sinLUT[MathUtils.round(x / Settings.SINCOS_LUT_PRECISION) % Settings.SINCOS_LUT_LENGTH];
    }
  }

  public static final float cos(float x) {
    if (Settings.SINCOS_LUT_ENABLED) {
      return sinLUT(HALF_PI - x);
    } else {
      return (float) StrictMath.cos(x);
    }
  }

  public static final float abs(final float x) {
    if (Settings.FAST_ABS) {
      return x > 0 ? x : -x;
    } else {
      return StrictMath.abs(x);
    }
  }

  public static final float fastAbs(final float x) {
    return x > 0 ? x : -x;
  }

  public static final int abs(int x) {
    int y = x >> 31;
    return (x ^ y) - y;
  }

  public static final int floor(final float x) {
    if (Settings.FAST_FLOOR) {
      return fastFloor(x);
    } else {
      return (int) StrictMath.floor(x);
    }
  }

  public static final int fastFloor(final float x) {
    int y = (int) x;
    if (x < y) {
      return y - 1;
    }
    return y;
  }

  public static final int ceil(final float x) {
    if (Settings.FAST_CEIL) {
      return fastCeil(x);
    } else {
      return (int) StrictMath.ceil(x);
    }
  }

  public static final int fastCeil(final float x) {
    int y = (int) x;
    if (x > y) {
      return y + 1;
    }
    return y;
  }

  public static final int round(final float x) {
    if (Settings.FAST_ROUND) {
      return floor(x + .5f);
    } else {
      return StrictMath.round(x);
    }
  }

  
  public static final int ceilPowerOf2(int x) {
    int pow2 = 1;
    while (pow2 < x) {
      pow2 <<= 1;
    }
    return pow2;
  }

  public final static float max(final float a, final float b) {
    return a > b ? a : b;
  }

  public final static int max(final int a, final int b) {
    return a > b ? a : b;
  }

  public final static float min(final float a, final float b) {
    return a < b ? a : b;
  }

  public final static int min(final int a, final int b) {
    return a < b ? a : b;
  }

  public final static float map(final float val, final float fromMin, final float fromMax,
      final float toMin, final float toMax) {
    final float mult = (val - fromMin) / (fromMax - fromMin);
    final float res = toMin + mult * (toMax - toMin);
    return res;
  }

  
  public final static float clamp(final float a, final float low, final float high) {
    return max(low, min(a, high));
  }

  public final static Vec2 clamp(final Vec2 a, final Vec2 low, final Vec2 high) {
    final Vec2 min = new Vec2();
    min.x = a.x < high.x ? a.x : high.x;
    min.y = a.y < high.y ? a.y : high.y;
    min.x = low.x > min.x ? low.x : min.x;
    min.y = low.y > min.y ? low.y : min.y;
    return min;
  }

  public final static void clampToOut(final Vec2 a, final Vec2 low, final Vec2 high, final Vec2 dest) {
    dest.x = a.x < high.x ? a.x : high.x;
    dest.y = a.y < high.y ? a.y : high.y;
    dest.x = low.x > dest.x ? low.x : dest.x;
    dest.y = low.y > dest.y ? low.y : dest.y;
  }

  
  public final static int nextPowerOfTwo(int x) {
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
  }

  public final static boolean isPowerOfTwo(final int x) {
    return x > 0 && (x & x - 1) == 0;
  }

  public static final float pow(float a, float b) {
    if (Settings.FAST_POW) {
      return fastPow(a, b);
    } else {
      return (float) StrictMath.pow(a, b);
    }
  }

  public static final float atan2(final float y, final float x) {
    if (Settings.FAST_ATAN2) {
      return fastAtan2(y, x);
    } else {
      return (float) StrictMath.atan2(y, x);
    }
  }

  public static final float fastAtan2(float y, float x) {
    if (x == 0.0f) {
      if (y > 0.0f) return HALF_PI;
      if (y == 0.0f) return 0.0f;
      return -HALF_PI;
    }
    float atan;
    final float z = y / x;
    if (abs(z) < 1.0f) {
      atan = z / (1.0f + 0.28f * z * z);
      if (x < 0.0f) {
        if (y < 0.0f) return atan - PI;
        return atan + PI;
      }
    } else {
      atan = HALF_PI - z / (z * z + 0.28f);
      if (y < 0.0f) return atan - PI;
    }
    return atan;
  }

  public static final float reduceAngle(float theta) {
    theta %= TWOPI;
    if (abs(theta) > PI) {
      theta = theta - TWOPI;
    }
    if (abs(theta) > HALF_PI) {
      theta = PI - theta;
    }
    return theta;
  }

  public static final float randomFloat(float argLow, float argHigh) {
    return (float) Math.random() * (argHigh - argLow) + argLow;
  }

  public static final float randomFloat(Random r, float argLow, float argHigh) {
    return r.nextFloat() * (argHigh - argLow) + argLow;
  }

  public static final float sqrt(float x) {
    return (float) StrictMath.sqrt(x);
  }

  public final static float distanceSquared(Vec2 v1, Vec2 v2) {
    float dx = (v1.x - v2.x);
    float dy = (v1.y - v2.y);
    return dx * dx + dy * dy;
  }

  public final static float distance(Vec2 v1, Vec2 v2) {
    return sqrt(distanceSquared(v1, v2));
  }
}