


package org.jbox2d.dynamics.joints;

import org.jbox2d.common.Vec2;
import org.jbox2d.dynamics.Body;


public class RevoluteJointDef extends JointDef {

  
  public Vec2 localAnchorA;

  
  public Vec2 localAnchorB;

  
  public float referenceAngle;

  
  public boolean enableLimit;

  
  public float lowerAngle;

  
  public float upperAngle;

  
  public boolean enableMotor;

  
  public float motorSpeed;

  
  public float maxMotorTorque;

  public RevoluteJointDef() {
    super(JointType.REVOLUTE);
    localAnchorA = new Vec2(0.0f, 0.0f);
    localAnchorB = new Vec2(0.0f, 0.0f);
    referenceAngle = 0.0f;
    lowerAngle = 0.0f;
    upperAngle = 0.0f;
    maxMotorTorque = 0.0f;
    motorSpeed = 0.0f;
    enableLimit = false;
    enableMotor = false;
  }

  
  public void initialize(final Body b1, final Body b2, final Vec2 anchor) {
    bodyA = b1;
    bodyB = b2;
    bodyA.getLocalPointToOut(anchor, localAnchorA);
    bodyB.getLocalPointToOut(anchor, localAnchorB);
    referenceAngle = bodyB.getAngle() - bodyA.getAngle();
  }
}
