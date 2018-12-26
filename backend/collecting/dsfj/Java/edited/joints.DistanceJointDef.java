


package org.jbox2d.dynamics.joints;

import org.jbox2d.common.Vec2;
import org.jbox2d.dynamics.Body;



public class DistanceJointDef extends JointDef {
  
  public final Vec2 localAnchorA;

  
  public final Vec2 localAnchorB;

  
  public float length;

  
  public float frequencyHz;

  
  public float dampingRatio;

  public DistanceJointDef() {
    super(JointType.DISTANCE);
    localAnchorA = new Vec2(0.0f, 0.0f);
    localAnchorB = new Vec2(0.0f, 0.0f);
    length = 1.0f;
    frequencyHz = 0.0f;
    dampingRatio = 0.0f;
  }

  
  public void initialize(final Body b1, final Body b2, final Vec2 anchor1, final Vec2 anchor2) {
    bodyA = b1;
    bodyB = b2;
    localAnchorA.set(bodyA.getLocalPoint(anchor1));
    localAnchorB.set(bodyB.getLocalPoint(anchor2));
    Vec2 d = anchor2.sub(anchor1);
    length = d.length();
  }
}
