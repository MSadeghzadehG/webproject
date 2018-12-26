
package org.jbox2d.common;

import java.io.Serializable;


public class Rot implements Serializable {
  private static final long serialVersionUID = 1L;

  public float s, c; 
  public Rot() {
    setIdentity();
  }

  public Rot(float angle) {
    set(angle);
  }

  public float getSin() {
    return s;
  }

  @Override
  public String toString() {
    return "Rot(s:" + s + ", c:" + c + ")";
  }

  public float getCos() {
    return c;
  }

  public Rot set(float angle) {
    s = MathUtils.sin(angle);
    c = MathUtils.cos(angle);
    return this;
  }

  public Rot set(Rot other) {
    s = other.s;
    c = other.c;
    return this;
  }

  public Rot setIdentity() {
    s = 0;
    c = 1;
    return this;
  }

  public float getAngle() {
    return MathUtils.atan2(s, c);
  }

  public void getXAxis(Vec2 xAxis) {
    xAxis.set(c, s);
  }

  public void getYAxis(Vec2 yAxis) {
    yAxis.set(-s, c);
  }

    public Rot clone() {
    Rot copy = new Rot();
    copy.s = s;
    copy.c = c;
    return copy;
  }

  public static final void mul(Rot q, Rot r, Rot out) {
    float tempc = q.c * r.c - q.s * r.s;
    out.s = q.s * r.c + q.c * r.s;
    out.c = tempc;
  }

  public static final void mulUnsafe(Rot q, Rot r, Rot out) {
    assert (r != out);
    assert (q != out);
                    out.s = q.s * r.c + q.c * r.s;
    out.c = q.c * r.c - q.s * r.s;
  }

  public static final void mulTrans(Rot q, Rot r, Rot out) {
    final float tempc = q.c * r.c + q.s * r.s;
    out.s = q.c * r.s - q.s * r.c;
    out.c = tempc;
  }

  public static final void mulTransUnsafe(Rot q, Rot r, Rot out) {
                    out.s = q.c * r.s - q.s * r.c;
    out.c = q.c * r.c + q.s * r.s;
  }

  public static final void mulToOut(Rot q, Vec2 v, Vec2 out) {
    float tempy = q.s * v.x + q.c * v.y;
    out.x = q.c * v.x - q.s * v.y;
    out.y = tempy;
  }

  public static final void mulToOutUnsafe(Rot q, Vec2 v, Vec2 out) {
    out.x = q.c * v.x - q.s * v.y;
    out.y = q.s * v.x + q.c * v.y;
  }

  public static final void mulTrans(Rot q, Vec2 v, Vec2 out) {
    final float tempy = -q.s * v.x + q.c * v.y;
    out.x = q.c * v.x + q.s * v.y;
    out.y = tempy;
  }

  public static final void mulTransUnsafe(Rot q, Vec2 v, Vec2 out) {
    out.x = q.c * v.x + q.s * v.y;
    out.y = -q.s * v.x + q.c * v.y;
  }
}
