
package org.jbox2d.dynamics;

import org.jbox2d.collision.broadphase.BroadPhase;
import org.jbox2d.collision.shapes.MassData;
import org.jbox2d.collision.shapes.Shape;
import org.jbox2d.common.MathUtils;
import org.jbox2d.common.Rot;
import org.jbox2d.common.Sweep;
import org.jbox2d.common.Transform;
import org.jbox2d.common.Vec2;
import org.jbox2d.dynamics.contacts.Contact;
import org.jbox2d.dynamics.contacts.ContactEdge;
import org.jbox2d.dynamics.joints.JointEdge;


public class Body {
  public static final int e_islandFlag = 0x0001;
  public static final int e_awakeFlag = 0x0002;
  public static final int e_autoSleepFlag = 0x0004;
  public static final int e_bulletFlag = 0x0008;
  public static final int e_fixedRotationFlag = 0x0010;
  public static final int e_activeFlag = 0x0020;
  public static final int e_toiFlag = 0x0040;

  public BodyType m_type;

  public int m_flags;

  public int m_islandIndex;

  
  public final Transform m_xf = new Transform();
  
  public final Transform m_xf0 = new Transform();

  
  public final Sweep m_sweep = new Sweep();

  public final Vec2 m_linearVelocity = new Vec2();
  public float m_angularVelocity = 0;

  public final Vec2 m_force = new Vec2();
  public float m_torque = 0;

  public World m_world;
  public Body m_prev;
  public Body m_next;

  public Fixture m_fixtureList;
  public int m_fixtureCount;

  public JointEdge m_jointList;
  public ContactEdge m_contactList;

  public float m_mass, m_invMass;

    public float m_I, m_invI;

  public float m_linearDamping;
  public float m_angularDamping;
  public float m_gravityScale;

  public float m_sleepTime;

  public Object m_userData;


  public Body(final BodyDef bd, World world) {
    assert (bd.position.isValid());
    assert (bd.linearVelocity.isValid());
    assert (bd.gravityScale >= 0.0f);
    assert (bd.angularDamping >= 0.0f);
    assert (bd.linearDamping >= 0.0f);

    m_flags = 0;

    if (bd.bullet) {
      m_flags |= e_bulletFlag;
    }
    if (bd.fixedRotation) {
      m_flags |= e_fixedRotationFlag;
    }
    if (bd.allowSleep) {
      m_flags |= e_autoSleepFlag;
    }
    if (bd.awake) {
      m_flags |= e_awakeFlag;
    }
    if (bd.active) {
      m_flags |= e_activeFlag;
    }

    m_world = world;

    m_xf.p.set(bd.position);
    m_xf.q.set(bd.angle);

    m_sweep.localCenter.setZero();
    m_sweep.c0.set(m_xf.p);
    m_sweep.c.set(m_xf.p);
    m_sweep.a0 = bd.angle;
    m_sweep.a = bd.angle;
    m_sweep.alpha0 = 0.0f;

    m_jointList = null;
    m_contactList = null;
    m_prev = null;
    m_next = null;

    m_linearVelocity.set(bd.linearVelocity);
    m_angularVelocity = bd.angularVelocity;

    m_linearDamping = bd.linearDamping;
    m_angularDamping = bd.angularDamping;
    m_gravityScale = bd.gravityScale;

    m_force.setZero();
    m_torque = 0.0f;

    m_sleepTime = 0.0f;

    m_type = bd.type;

    if (m_type == BodyType.DYNAMIC) {
      m_mass = 1f;
      m_invMass = 1f;
    } else {
      m_mass = 0f;
      m_invMass = 0f;
    }

    m_I = 0.0f;
    m_invI = 0.0f;

    m_userData = bd.userData;

    m_fixtureList = null;
    m_fixtureCount = 0;
  }

  
  public final Fixture createFixture(FixtureDef def) {
    assert (m_world.isLocked() == false);

    if (m_world.isLocked() == true) {
      return null;
    }

    Fixture fixture = new Fixture();
    fixture.create(this, def);

    if ((m_flags & e_activeFlag) == e_activeFlag) {
      BroadPhase broadPhase = m_world.m_contactManager.m_broadPhase;
      fixture.createProxies(broadPhase, m_xf);
    }

    fixture.m_next = m_fixtureList;
    m_fixtureList = fixture;
    ++m_fixtureCount;

    fixture.m_body = this;

        if (fixture.m_density > 0.0f) {
      resetMassData();
    }

            m_world.m_flags |= World.NEW_FIXTURE;

    return fixture;
  }

  private final FixtureDef fixDef = new FixtureDef();

  
  public final Fixture createFixture(Shape shape, float density) {
    fixDef.shape = shape;
    fixDef.density = density;

    return createFixture(fixDef);
  }

  
  public final void destroyFixture(Fixture fixture) {
    assert (m_world.isLocked() == false);
    if (m_world.isLocked() == true) {
      return;
    }

    assert (fixture.m_body == this);

        assert (m_fixtureCount > 0);
    Fixture node = m_fixtureList;
    Fixture last = null;     boolean found = false;
    while (node != null) {
      if (node == fixture) {
        node = fixture.m_next;
        found = true;
        break;
      }
      last = node;
      node = node.m_next;
    }

        assert (found);

        if (last == null) {
      m_fixtureList = fixture.m_next;
    } else {
      last.m_next = fixture.m_next;
    }

        ContactEdge edge = m_contactList;
    while (edge != null) {
      Contact c = edge.contact;
      edge = edge.next;

      Fixture fixtureA = c.getFixtureA();
      Fixture fixtureB = c.getFixtureB();

      if (fixture == fixtureA || fixture == fixtureB) {
                        m_world.m_contactManager.destroy(c);
      }
    }

    if ((m_flags & e_activeFlag) == e_activeFlag) {
      BroadPhase broadPhase = m_world.m_contactManager.m_broadPhase;
      fixture.destroyProxies(broadPhase);
    }

    fixture.destroy();
    fixture.m_body = null;
    fixture.m_next = null;
    fixture = null;

    --m_fixtureCount;

        resetMassData();
  }

  
  public final void setTransform(Vec2 position, float angle) {
    assert (m_world.isLocked() == false);
    if (m_world.isLocked() == true) {
      return;
    }

    m_xf.q.set(angle);
    m_xf.p.set(position);

        Transform.mulToOutUnsafe(m_xf, m_sweep.localCenter, m_sweep.c);
    m_sweep.a = angle;

    m_sweep.c0.set(m_sweep.c);
    m_sweep.a0 = m_sweep.a;

    BroadPhase broadPhase = m_world.m_contactManager.m_broadPhase;
    for (Fixture f = m_fixtureList; f != null; f = f.m_next) {
      f.synchronize(broadPhase, m_xf, m_xf);
    }
  }

  
  public final Transform getTransform() {
    return m_xf;
  }

  
  public final Vec2 getPosition() {
    return m_xf.p;
  }

  
  public final float getAngle() {
    return m_sweep.a;
  }

  
  public final Vec2 getWorldCenter() {
    return m_sweep.c;
  }

  
  public final Vec2 getLocalCenter() {
    return m_sweep.localCenter;
  }

  
  public final void setLinearVelocity(Vec2 v) {
    if (m_type == BodyType.STATIC) {
      return;
    }

    if (Vec2.dot(v, v) > 0.0f) {
      setAwake(true);
    }

    m_linearVelocity.set(v);
  }

  
  public final Vec2 getLinearVelocity() {
    return m_linearVelocity;
  }

  
  public final void setAngularVelocity(float w) {
    if (m_type == BodyType.STATIC) {
      return;
    }

    if (w * w > 0f) {
      setAwake(true);
    }

    m_angularVelocity = w;
  }

  
  public final float getAngularVelocity() {
    return m_angularVelocity;
  }

  
  public float getGravityScale() {
    return m_gravityScale;
  }

  
  public void setGravityScale(float gravityScale) {
    this.m_gravityScale = gravityScale;
  }

  
  public final void applyForce(Vec2 force, Vec2 point) {
    if (m_type != BodyType.DYNAMIC) {
      return;
    }

    if (isAwake() == false) {
      setAwake(true);
    }

                
    m_force.x += force.x;
    m_force.y += force.y;

    m_torque += (point.x - m_sweep.c.x) * force.y - (point.y - m_sweep.c.y) * force.x;
  }

  
  public final void applyForceToCenter(Vec2 force) {
    if (m_type != BodyType.DYNAMIC) {
      return;
    }

    if (isAwake() == false) {
      setAwake(true);
    }

    m_force.x += force.x;
    m_force.y += force.y;
  }

  
  public final void applyTorque(float torque) {
    if (m_type != BodyType.DYNAMIC) {
      return;
    }

    if (isAwake() == false) {
      setAwake(true);
    }

    m_torque += torque;
  }

  
  public final void applyLinearImpulse(Vec2 impulse, Vec2 point, boolean wake) {
    if (m_type != BodyType.DYNAMIC) {
      return;
    }

    if (!isAwake()) {
      if (wake) {
        setAwake(true);
      } else {
        return;
      }
    }

    m_linearVelocity.x += impulse.x * m_invMass;
    m_linearVelocity.y += impulse.y * m_invMass;

    m_angularVelocity +=
        m_invI * ((point.x - m_sweep.c.x) * impulse.y - (point.y - m_sweep.c.y) * impulse.x);
  }

  
  public void applyAngularImpulse(float impulse) {
    if (m_type != BodyType.DYNAMIC) {
      return;
    }

    if (isAwake() == false) {
      setAwake(true);
    }
    m_angularVelocity += m_invI * impulse;
  }

  
  public final float getMass() {
    return m_mass;
  }

  
  public final float getInertia() {
    return m_I
        + m_mass
        * (m_sweep.localCenter.x * m_sweep.localCenter.x + m_sweep.localCenter.y
            * m_sweep.localCenter.y);
  }

  
  public final void getMassData(MassData data) {
            
    data.mass = m_mass;
    data.I =
        m_I
            + m_mass
            * (m_sweep.localCenter.x * m_sweep.localCenter.x + m_sweep.localCenter.y
                * m_sweep.localCenter.y);
    data.center.x = m_sweep.localCenter.x;
    data.center.y = m_sweep.localCenter.y;
  }

  
  public final void setMassData(MassData massData) {
        assert (m_world.isLocked() == false);
    if (m_world.isLocked() == true) {
      return;
    }

    if (m_type != BodyType.DYNAMIC) {
      return;
    }

    m_invMass = 0.0f;
    m_I = 0.0f;
    m_invI = 0.0f;

    m_mass = massData.mass;
    if (m_mass <= 0.0f) {
      m_mass = 1f;
    }

    m_invMass = 1.0f / m_mass;

    if (massData.I > 0.0f && (m_flags & e_fixedRotationFlag) == 0) {
      m_I = massData.I - m_mass * Vec2.dot(massData.center, massData.center);
      assert (m_I > 0.0f);
      m_invI = 1.0f / m_I;
    }

    final Vec2 oldCenter = m_world.getPool().popVec2();
        oldCenter.set(m_sweep.c);
    m_sweep.localCenter.set(massData.center);
        Transform.mulToOutUnsafe(m_xf, m_sweep.localCenter, m_sweep.c0);
    m_sweep.c.set(m_sweep.c0);

            final Vec2 temp = m_world.getPool().popVec2();
    temp.set(m_sweep.c).subLocal(oldCenter);
    Vec2.crossToOut(m_angularVelocity, temp, temp);
    m_linearVelocity.addLocal(temp);

    m_world.getPool().pushVec2(2);
  }

  private final MassData pmd = new MassData();

  
  public final void resetMassData() {
        m_mass = 0.0f;
    m_invMass = 0.0f;
    m_I = 0.0f;
    m_invI = 0.0f;
    m_sweep.localCenter.setZero();

        if (m_type == BodyType.STATIC || m_type == BodyType.KINEMATIC) {
            m_sweep.c0.set(m_xf.p);
      m_sweep.c.set(m_xf.p);
      m_sweep.a0 = m_sweep.a;
      return;
    }

    assert (m_type == BodyType.DYNAMIC);

        final Vec2 localCenter = m_world.getPool().popVec2();
    localCenter.setZero();
    final Vec2 temp = m_world.getPool().popVec2();
    final MassData massData = pmd;
    for (Fixture f = m_fixtureList; f != null; f = f.m_next) {
      if (f.m_density == 0.0f) {
        continue;
      }
      f.getMassData(massData);
      m_mass += massData.mass;
            temp.set(massData.center).mulLocal(massData.mass);
      localCenter.addLocal(temp);
      m_I += massData.I;
    }

        if (m_mass > 0.0f) {
      m_invMass = 1.0f / m_mass;
      localCenter.mulLocal(m_invMass);
    } else {
            m_mass = 1.0f;
      m_invMass = 1.0f;
    }

    if (m_I > 0.0f && (m_flags & e_fixedRotationFlag) == 0) {
            m_I -= m_mass * Vec2.dot(localCenter, localCenter);
      assert (m_I > 0.0f);
      m_invI = 1.0f / m_I;
    } else {
      m_I = 0.0f;
      m_invI = 0.0f;
    }

    Vec2 oldCenter = m_world.getPool().popVec2();
        oldCenter.set(m_sweep.c);
    m_sweep.localCenter.set(localCenter);
        Transform.mulToOutUnsafe(m_xf, m_sweep.localCenter, m_sweep.c0);
    m_sweep.c.set(m_sweep.c0);

            temp.set(m_sweep.c).subLocal(oldCenter);

    final Vec2 temp2 = oldCenter;
    Vec2.crossToOutUnsafe(m_angularVelocity, temp, temp2);
    m_linearVelocity.addLocal(temp2);

    m_world.getPool().pushVec2(3);
  }

  
  public final Vec2 getWorldPoint(Vec2 localPoint) {
    Vec2 v = new Vec2();
    getWorldPointToOut(localPoint, v);
    return v;
  }

  public final void getWorldPointToOut(Vec2 localPoint, Vec2 out) {
    Transform.mulToOut(m_xf, localPoint, out);
  }

  
  public final Vec2 getWorldVector(Vec2 localVector) {
    Vec2 out = new Vec2();
    getWorldVectorToOut(localVector, out);
    return out;
  }

  public final void getWorldVectorToOut(Vec2 localVector, Vec2 out) {
    Rot.mulToOut(m_xf.q, localVector, out);
  }

  public final void getWorldVectorToOutUnsafe(Vec2 localVector, Vec2 out) {
    Rot.mulToOutUnsafe(m_xf.q, localVector, out);
  }

  
  public final Vec2 getLocalPoint(Vec2 worldPoint) {
    Vec2 out = new Vec2();
    getLocalPointToOut(worldPoint, out);
    return out;
  }

  public final void getLocalPointToOut(Vec2 worldPoint, Vec2 out) {
    Transform.mulTransToOut(m_xf, worldPoint, out);
  }

  
  public final Vec2 getLocalVector(Vec2 worldVector) {
    Vec2 out = new Vec2();
    getLocalVectorToOut(worldVector, out);
    return out;
  }

  public final void getLocalVectorToOut(Vec2 worldVector, Vec2 out) {
    Rot.mulTrans(m_xf.q, worldVector, out);
  }

  public final void getLocalVectorToOutUnsafe(Vec2 worldVector, Vec2 out) {
    Rot.mulTransUnsafe(m_xf.q, worldVector, out);
  }

  
  public final Vec2 getLinearVelocityFromWorldPoint(Vec2 worldPoint) {
    Vec2 out = new Vec2();
    getLinearVelocityFromWorldPointToOut(worldPoint, out);
    return out;
  }

  public final void getLinearVelocityFromWorldPointToOut(Vec2 worldPoint, Vec2 out) {
    final float tempX = worldPoint.x - m_sweep.c.x;
    final float tempY = worldPoint.y - m_sweep.c.y;
    out.x = -m_angularVelocity * tempY + m_linearVelocity.x;
    out.y = m_angularVelocity * tempX + m_linearVelocity.y;
  }

  
  public final Vec2 getLinearVelocityFromLocalPoint(Vec2 localPoint) {
    Vec2 out = new Vec2();
    getLinearVelocityFromLocalPointToOut(localPoint, out);
    return out;
  }

  public final void getLinearVelocityFromLocalPointToOut(Vec2 localPoint, Vec2 out) {
    getWorldPointToOut(localPoint, out);
    getLinearVelocityFromWorldPointToOut(out, out);
  }

  
  public final float getLinearDamping() {
    return m_linearDamping;
  }

  
  public final void setLinearDamping(float linearDamping) {
    m_linearDamping = linearDamping;
  }

  
  public final float getAngularDamping() {
    return m_angularDamping;
  }

  
  public final void setAngularDamping(float angularDamping) {
    m_angularDamping = angularDamping;
  }

  public BodyType getType() {
    return m_type;
  }

  
  public void setType(BodyType type) {
    assert (m_world.isLocked() == false);
    if (m_world.isLocked() == true) {
      return;
    }

    if (m_type == type) {
      return;
    }

    m_type = type;

    resetMassData();

    if (m_type == BodyType.STATIC) {
      m_linearVelocity.setZero();
      m_angularVelocity = 0.0f;
      m_sweep.a0 = m_sweep.a;
      m_sweep.c0.set(m_sweep.c);
      synchronizeFixtures();
    }

    setAwake(true);

    m_force.setZero();
    m_torque = 0.0f;

        ContactEdge ce = m_contactList;
    while (ce != null) {
      ContactEdge ce0 = ce;
      ce = ce.next;
      m_world.m_contactManager.destroy(ce0.contact);
    }
    m_contactList = null;

        BroadPhase broadPhase = m_world.m_contactManager.m_broadPhase;
    for (Fixture f = m_fixtureList; f != null; f = f.m_next) {
      int proxyCount = f.m_proxyCount;
      for (int i = 0; i < proxyCount; ++i) {
        broadPhase.touchProxy(f.m_proxies[i].proxyId);
      }
    }
  }

  
  public final boolean isBullet() {
    return (m_flags & e_bulletFlag) == e_bulletFlag;
  }

  
  public final void setBullet(boolean flag) {
    if (flag) {
      m_flags |= e_bulletFlag;
    } else {
      m_flags &= ~e_bulletFlag;
    }
  }

  
  public void setSleepingAllowed(boolean flag) {
    if (flag) {
      m_flags |= e_autoSleepFlag;
    } else {
      m_flags &= ~e_autoSleepFlag;
      setAwake(true);
    }
  }

  
  public boolean isSleepingAllowed() {
    return (m_flags & e_autoSleepFlag) == e_autoSleepFlag;
  }

  
  public void setAwake(boolean flag) {
    if (flag) {
      if ((m_flags & e_awakeFlag) == 0) {
        m_flags |= e_awakeFlag;
        m_sleepTime = 0.0f;
      }
    } else {
      m_flags &= ~e_awakeFlag;
      m_sleepTime = 0.0f;
      m_linearVelocity.setZero();
      m_angularVelocity = 0.0f;
      m_force.setZero();
      m_torque = 0.0f;
    }
  }

  
  public boolean isAwake() {
    return (m_flags & e_awakeFlag) == e_awakeFlag;
  }

  
  public void setActive(boolean flag) {
    assert (m_world.isLocked() == false);

    if (flag == isActive()) {
      return;
    }

    if (flag) {
      m_flags |= e_activeFlag;

            BroadPhase broadPhase = m_world.m_contactManager.m_broadPhase;
      for (Fixture f = m_fixtureList; f != null; f = f.m_next) {
        f.createProxies(broadPhase, m_xf);
      }

          } else {
      m_flags &= ~e_activeFlag;

            BroadPhase broadPhase = m_world.m_contactManager.m_broadPhase;
      for (Fixture f = m_fixtureList; f != null; f = f.m_next) {
        f.destroyProxies(broadPhase);
      }

            ContactEdge ce = m_contactList;
      while (ce != null) {
        ContactEdge ce0 = ce;
        ce = ce.next;
        m_world.m_contactManager.destroy(ce0.contact);
      }
      m_contactList = null;
    }
  }

  
  public boolean isActive() {
    return (m_flags & e_activeFlag) == e_activeFlag;
  }

  
  public void setFixedRotation(boolean flag) {
    if (flag) {
      m_flags |= e_fixedRotationFlag;
    } else {
      m_flags &= ~e_fixedRotationFlag;
    }

    resetMassData();
  }

  
  public boolean isFixedRotation() {
    return (m_flags & e_fixedRotationFlag) == e_fixedRotationFlag;
  }

  
  public final Fixture getFixtureList() {
    return m_fixtureList;
  }

  
  public final JointEdge getJointList() {
    return m_jointList;
  }

  
  public final ContactEdge getContactList() {
    return m_contactList;
  }

  
  public final Body getNext() {
    return m_next;
  }

  
  public final Object getUserData() {
    return m_userData;
  }

  
  public final void setUserData(Object data) {
    m_userData = data;
  }

  
  public final World getWorld() {
    return m_world;
  }

    private final Transform pxf = new Transform();

  protected final void synchronizeFixtures() {
    final Transform xf1 = pxf;
    
                    xf1.q.s = MathUtils.sin(m_sweep.a0);
    xf1.q.c = MathUtils.cos(m_sweep.a0);
    xf1.p.x = m_sweep.c0.x - xf1.q.c * m_sweep.localCenter.x + xf1.q.s * m_sweep.localCenter.y;
    xf1.p.y = m_sweep.c0.y - xf1.q.s * m_sweep.localCenter.x - xf1.q.c * m_sweep.localCenter.y;
    
    for (Fixture f = m_fixtureList; f != null; f = f.m_next) {
      f.synchronize(m_world.m_contactManager.m_broadPhase, xf1, m_xf);
    }
  }

  public final void synchronizeTransform() {
                            m_xf.q.s = MathUtils.sin(m_sweep.a);
    m_xf.q.c = MathUtils.cos(m_sweep.a);
    Rot q = m_xf.q;
    Vec2 v = m_sweep.localCenter;
    m_xf.p.x = m_sweep.c.x - q.c * v.x + q.s * v.y;
    m_xf.p.y = m_sweep.c.y - q.s * v.x - q.c * v.y;
  }

  
  public boolean shouldCollide(Body other) {
        if (m_type != BodyType.DYNAMIC && other.m_type != BodyType.DYNAMIC) {
      return false;
    }

        for (JointEdge jn = m_jointList; jn != null; jn = jn.next) {
      if (jn.other == other) {
        if (jn.joint.getCollideConnected() == false) {
          return false;
        }
      }
    }

    return true;
  }

  protected final void advance(float t) {
        m_sweep.advance(t);
    m_sweep.c.set(m_sweep.c0);
    m_sweep.a = m_sweep.a0;
    m_xf.q.set(m_sweep.a);
        Rot.mulToOutUnsafe(m_xf.q, m_sweep.localCenter, m_xf.p);
    m_xf.p.mulLocal(-1).addLocal(m_sweep.c);
  }
}
