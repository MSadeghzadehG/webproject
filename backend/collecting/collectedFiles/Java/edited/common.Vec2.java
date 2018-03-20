
package org.jbox2d.common;

import java.io.Serializable;


public class Vec2 implements Serializable {
  private static final long serialVersionUID = 1L;

  public float x, y;

  public Vec2() {
    this(0, 0);
  }

  public Vec2(float x, float y) {
    this.x = x;
    this.y = y;
  }

  public Vec2(Vec2 toCopy) {
    this(toCopy.x, toCopy.y);
  }

  
  public final void setZero() {
    x = 0.0f;
    y = 0.0f;
  }

  
  public final Vec2 set(float x, float y) {
    this.x = x;
    this.y = y;
    return this;
  }

  
  public final Vec2 set(Vec2 v) {
    this.x = v.x;
    this.y = v.y;
    return this;
  }

  
  public final Vec2 add(Vec2 v) {
    return new Vec2(x + v.x, y + v.y);
  }



  
  public final Vec2 sub(Vec2 v) {
    return new Vec2(x - v.x, y - v.y);
  }

  
  public final Vec2 mul(float a) {
    return new Vec2(x * a, y * a);
  }

  
  public final Vec2 negate() {
    return new Vec2(-x, -y);
  }

  
  public final Vec2 negateLocal() {
    x = -x;
    y = -y;
    return this;
  }

  
  public final Vec2 addLocal(Vec2 v) {
    x += v.x;
    y += v.y;
    return this;
  }

  
  public final Vec2 addLocal(float x, float y) {
    this.x += x;
    this.y += y;
    return this;
  }

  
  public final Vec2 subLocal(Vec2 v) {
    x -= v.x;
    y -= v.y;
    return this;
  }

  
  public final Vec2 mulLocal(float a) {
    x *= a;
    y *= a;
    return this;
  }

  
  public final Vec2 skew() {
    return new Vec2(-y, x);
  }

  
  public final void skew(Vec2 out) {
    out.x = -y;
    out.y = x;
  }

  
  public final float length() {
    return MathUtils.sqrt(x * x + y * y);
  }

  
  public final float lengthSquared() {
    return (x * x + y * y);
  }

  
  public final float normalize() {
    float length = length();
    if (length < Settings.EPSILON) {
      return 0f;
    }

    float invLength = 1.0f / length;
    x *= invLength;
    y *= invLength;
    return length;
  }

  
  public final boolean isValid() {
    return !Float.isNaN(x) && !Float.isInfinite(x) && !Float.isNaN(y) && !Float.isInfinite(y);
  }

  
  public final Vec2 abs() {
    return new Vec2(MathUtils.abs(x), MathUtils.abs(y));
  }

  public final void absLocal() {
    x = MathUtils.abs(x);
    y = MathUtils.abs(y);
  }

    
  public final Vec2 clone() {
    return new Vec2(x, y);
  }

  @Override
  public final String toString() {
    return "(" + x + "," + y + ")";
  }

  

  public final static Vec2 abs(Vec2 a) {
    return new Vec2(MathUtils.abs(a.x), MathUtils.abs(a.y));
  }

  public final static void absToOut(Vec2 a, Vec2 out) {
    out.x = MathUtils.abs(a.x);
    out.y = MathUtils.abs(a.y);
  }

  public final static float dot(final Vec2 a, final Vec2 b) {
    return a.x * b.x + a.y * b.y;
  }

  public final static float cross(final Vec2 a, final Vec2 b) {
    return a.x * b.y - a.y * b.x;
  }

  public final static Vec2 cross(Vec2 a, float s) {
    return new Vec2(s * a.y, -s * a.x);
  }

  public final static void crossToOut(Vec2 a, float s, Vec2 out) {
    final float tempy = -s * a.x;
    out.x = s * a.y;
    out.y = tempy;
  }

  public final static void crossToOutUnsafe(Vec2 a, float s, Vec2 out) {
    assert (out != a);
    out.x = s * a.y;
    out.y = -s * a.x;
  }

  public final static Vec2 cross(float s, Vec2 a) {
    return new Vec2(-s * a.y, s * a.x);
  }

  public final static void crossToOut(float s, Vec2 a, Vec2 out) {
    final float tempY = s * a.x;
    out.x = -s * a.y;
    out.y = tempY;
  }

  public final static void crossToOutUnsafe(float s, Vec2 a, Vec2 out) {
    assert (out != a);
    out.x = -s * a.y;
    out.y = s * a.x;
  }

  public final static void negateToOut(Vec2 a, Vec2 out) {
    out.x = -a.x;
    out.y = -a.y;
  }

  public final static Vec2 min(Vec2 a, Vec2 b) {
    return new Vec2(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y);
  }

  public final static Vec2 max(Vec2 a, Vec2 b) {
    return new Vec2(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y);
  }

  public final static void minToOut(Vec2 a, Vec2 b, Vec2 out) {
    out.x = a.x < b.x ? a.x : b.x;
    out.y = a.y < b.y ? a.y : b.y;
  }

  public final static void maxToOut(Vec2 a, Vec2 b, Vec2 out) {
    out.x = a.x > b.x ? a.x : b.x;
    out.y = a.y > b.y ? a.y : b.y;
  }

  
  @Override
  public int hashCode() {     final int prime = 31;
    int result = 1;
    result = prime * result + Float.floatToIntBits(x);
    result = prime * result + Float.floatToIntBits(y);
    return result;
  }

  
  @Override
  public boolean equals(Object obj) {     if (this == obj) return true;
    if (obj == null) return false;
    if (getClass() != obj.getClass()) return false;
    Vec2 other = (Vec2) obj;
    if (Float.floatToIntBits(x) != Float.floatToIntBits(other.x)) return false;
    if (Float.floatToIntBits(y) != Float.floatToIntBits(other.y)) return false;
    return true;
  }
}
