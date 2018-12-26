
package org.jbox2d.common;

import java.io.Serializable;



public class Transform implements Serializable {
  private static final long serialVersionUID = 1L;

  
  public final Vec2 p;

  
  public final Rot q;

  
  public Transform() {
    p = new Vec2();
    q = new Rot();
  }

  
  public Transform(final Transform xf) {
    p = xf.p.clone();
    q = xf.q.clone();
  }

  
  public Transform(final Vec2 _position, final Rot _R) {
    p = _position.clone();
    q = _R.clone();
  }

  
  public final Transform set(final Transform xf) {
    p.set(xf.p);
    q.set(xf.q);
    return this;
  }

  
  public final void set(Vec2 p, float angle) {
    this.p.set(p);
    q.set(angle);
  }

  
  public final void setIdentity() {
    p.setZero();
    q.setIdentity();
  }

  public final static Vec2 mul(final Transform T, final Vec2 v) {
    return new Vec2((T.q.c * v.x - T.q.s * v.y) + T.p.x, (T.q.s * v.x + T.q.c * v.y) + T.p.y);
  }

  public final static void mulToOut(final Transform T, final Vec2 v, final Vec2 out) {
    final float tempy = (T.q.s * v.x + T.q.c * v.y) + T.p.y;
    out.x = (T.q.c * v.x - T.q.s * v.y) + T.p.x;
    out.y = tempy;
  }

  public final static void mulToOutUnsafe(final Transform T, final Vec2 v, final Vec2 out) {
    assert (v != out);
    out.x = (T.q.c * v.x - T.q.s * v.y) + T.p.x;
    out.y = (T.q.s * v.x + T.q.c * v.y) + T.p.y;
  }

  public final static Vec2 mulTrans(final Transform T, final Vec2 v) {
    final float px = v.x - T.p.x;
    final float py = v.y - T.p.y;
    return new Vec2((T.q.c * px + T.q.s * py), (-T.q.s * px + T.q.c * py));
  }

  public final static void mulTransToOut(final Transform T, final Vec2 v, final Vec2 out) {
    final float px = v.x - T.p.x;
    final float py = v.y - T.p.y;
    final float tempy = (-T.q.s * px + T.q.c * py);
    out.x = (T.q.c * px + T.q.s * py);
    out.y = tempy;
  }
  
  public final static void mulTransToOutUnsafe(final Transform T, final Vec2 v, final Vec2 out) {
    assert(v != out);
    final float px = v.x - T.p.x;
    final float py = v.y - T.p.y;
    out.x = (T.q.c * px + T.q.s * py);
    out.y = (-T.q.s * px + T.q.c * py);
  }

  public final static Transform mul(final Transform A, final Transform B) {
    Transform C = new Transform();
    Rot.mulUnsafe(A.q, B.q, C.q);
    Rot.mulToOutUnsafe(A.q, B.p, C.p);
    C.p.addLocal(A.p);
    return C;
  }

  public final static void mulToOut(final Transform A, final Transform B, final Transform out) {
    assert (out != A);
    Rot.mul(A.q, B.q, out.q);
    Rot.mulToOut(A.q, B.p, out.p);
    out.p.addLocal(A.p);
  }

  public final static void mulToOutUnsafe(final Transform A, final Transform B, final Transform out) {
    assert (out != B);
    assert (out != A);
    Rot.mulUnsafe(A.q, B.q, out.q);
    Rot.mulToOutUnsafe(A.q, B.p, out.p);
    out.p.addLocal(A.p);
  }

  private static Vec2 pool = new Vec2();

  public final static Transform mulTrans(final Transform A, final Transform B) {
    Transform C = new Transform();
    Rot.mulTransUnsafe(A.q, B.q, C.q);
    pool.set(B.p).subLocal(A.p);
    Rot.mulTransUnsafe(A.q, pool, C.p);
    return C;
  }

  public final static void mulTransToOut(final Transform A, final Transform B, final Transform out) {
    assert (out != A);
    Rot.mulTrans(A.q, B.q, out.q);
    pool.set(B.p).subLocal(A.p);
    Rot.mulTrans(A.q, pool, out.p);
  }

  public final static void mulTransToOutUnsafe(final Transform A, final Transform B,
      final Transform out) {
    assert (out != A);
    assert (out != B);
    Rot.mulTransUnsafe(A.q, B.q, out.q);
    pool.set(B.p).subLocal(A.p);
    Rot.mulTransUnsafe(A.q, pool, out.p);
  }

  @Override
  public final String toString() {
    String s = "XForm:\n";
    s += "Position: " + p + "\n";
    s += "R: \n" + q + "\n";
    return s;
  }
}
