
package org.jbox2d.common;

import java.io.Serializable;


public class Sweep implements Serializable {
  private static final long serialVersionUID = 1L;

  
  public final Vec2 localCenter;
  
  public final Vec2 c0, c;
  
  public float a0, a;

  
  public float alpha0;

  public String toString() {
    String s = "Sweep:\nlocalCenter: " + localCenter + "\n";
    s += "c0: " + c0 + ", c: " + c + "\n";
    s += "a0: " + a0 + ", a: " + a + "\n";
    s += "alpha0: " + alpha0;
    return s;
  }

  public Sweep() {
    localCenter = new Vec2();
    c0 = new Vec2();
    c = new Vec2();
  }

  public final void normalize() {
    float d = MathUtils.TWOPI * MathUtils.floor(a0 / MathUtils.TWOPI);
    a0 -= d;
    a -= d;
  }

  public final Sweep set(Sweep other) {
    localCenter.set(other.localCenter);
    c0.set(other.c0);
    c.set(other.c);
    a0 = other.a0;
    a = other.a;
    alpha0 = other.alpha0;
    return this;
  }

  
  public final void getTransform(final Transform xf, final float beta) {
    assert (xf != null);
                xf.p.x = (1.0f - beta) * c0.x + beta * c.x;
    xf.p.y = (1.0f - beta) * c0.y + beta * c.y;
    float angle = (1.0f - beta) * a0 + beta * a;
    xf.q.set(angle);

            final Rot q = xf.q;
    xf.p.x -= q.c * localCenter.x - q.s * localCenter.y;
    xf.p.y -= q.s * localCenter.x + q.c * localCenter.y;
  }

  
  public final void advance(final float alpha) {
    assert(alpha0 < 1.0f);
                    float beta = (alpha - alpha0) / (1.0f - alpha0);
    c0.x += beta * (c.x - c0.x);
    c0.y += beta * (c.y - c0.y);
    a0 += beta * (a - a0);
    alpha0 = alpha;
  }
}
