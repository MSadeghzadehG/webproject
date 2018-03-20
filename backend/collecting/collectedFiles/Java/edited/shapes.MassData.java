


package org.jbox2d.collision.shapes;

import org.jbox2d.common.Vec2;



public class MassData {
	
	public float mass;
	
	public final Vec2 center;
	
	public float I;
	
	
	public MassData() {
		mass = I = 0f;
		center = new Vec2();
	}
	
	
	public MassData(MassData md) {
		mass = md.mass;
		I = md.I;
		center = md.center.clone();
	}
	
	public void set(MassData md) {
		mass = md.mass;
		I = md.I;
		center.set(md.center);
	}
	
	
	public MassData clone() {
		return new MassData(this);
	}
}
