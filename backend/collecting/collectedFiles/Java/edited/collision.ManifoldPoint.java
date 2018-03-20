


package org.jbox2d.collision;

import org.jbox2d.common.Vec2;


public class ManifoldPoint {
	
	public final Vec2 localPoint;
	
	public float normalImpulse;
	
	public float tangentImpulse;
	
	public final ContactID id;

	
	public ManifoldPoint() {
		localPoint = new Vec2();
		normalImpulse = tangentImpulse = 0f;
		id = new ContactID();
	}

	
	public ManifoldPoint(final ManifoldPoint cp) {
		localPoint = cp.localPoint.clone();
		normalImpulse = cp.normalImpulse;
		tangentImpulse = cp.tangentImpulse;
		id = new ContactID(cp.id);
	}

	
	public void set(final ManifoldPoint cp){
		localPoint.set(cp.localPoint);
		normalImpulse = cp.normalImpulse;
		tangentImpulse = cp.tangentImpulse;
		id.set(cp.id);
	}
}
