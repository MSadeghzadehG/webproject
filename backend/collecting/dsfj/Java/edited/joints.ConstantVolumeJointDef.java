
package org.jbox2d.dynamics.joints;

import java.util.ArrayList;

import org.jbox2d.dynamics.Body;


public class ConstantVolumeJointDef extends JointDef {
  public float frequencyHz;
  public float dampingRatio;

  ArrayList<Body> bodies;
  ArrayList<DistanceJoint> joints;

  public ConstantVolumeJointDef() {
    super(JointType.CONSTANT_VOLUME);
    bodies = new ArrayList<Body>();
    joints = null;
    collideConnected = false;
    frequencyHz = 0.0f;
    dampingRatio = 0.0f;
  }

  
  public void addBody(Body argBody) {
    bodies.add(argBody);
    if (bodies.size() == 1) {
      bodyA = argBody;
    }
    if (bodies.size() == 2) {
      bodyB = argBody;
    }
  }

  
  public void addBodyAndJoint(Body argBody, DistanceJoint argJoint) {
    addBody(argBody);
    if (joints == null) {
      joints = new ArrayList<DistanceJoint>();
    }
    joints.add(argJoint);
  }
}
