

package com.badlogic.gdx.math.collision;

import java.io.Serializable;

import com.badlogic.gdx.math.Matrix4;
import com.badlogic.gdx.math.Vector3;


public class Ray implements Serializable {
	private static final long serialVersionUID = -620692054835390878L;
	public final Vector3 origin = new Vector3();
	public final Vector3 direction = new Vector3();

	public Ray () { }
	
	
	public Ray (Vector3 origin, Vector3 direction) {
		this.origin.set(origin);
		this.direction.set(direction).nor();
	}

	
	public Ray cpy () {
		return new Ray(this.origin, this.direction);
	}

	
	public Vector3 getEndPoint (final Vector3 out, final float distance) {
		return out.set(direction).scl(distance).add(origin);
	}

	static Vector3 tmp = new Vector3();

	
	public Ray mul (Matrix4 matrix) {
		tmp.set(origin).add(direction);
		tmp.mul(matrix);
		origin.mul(matrix);
		direction.set(tmp.sub(origin));
		return this;
	}

	
	public String toString () {
		return "ray [" + origin + ":" + direction + "]";
	}

	
	public Ray set (Vector3 origin, Vector3 direction) {
		this.origin.set(origin);
		this.direction.set(direction);
		return this;
	}

	
	public Ray set (float x, float y, float z, float dx, float dy, float dz) {
		this.origin.set(x, y, z);
		this.direction.set(dx, dy, dz);
		return this;
	}

	
	public Ray set (Ray ray) {
		this.origin.set(ray.origin);
		this.direction.set(ray.direction);
		return this;
	}

	@Override
	public boolean equals (Object o) {
		if (o == this) return true;
		if (o == null || o.getClass() != this.getClass()) return false;
		Ray r = (Ray)o;
		return this.direction.equals(r.direction) && this.origin.equals(r.origin);
	}

	@Override
	public int hashCode () {
		final int prime = 73;
		int result = 1;
		result = prime * result + this.direction.hashCode();
		result = prime * result + this.origin.hashCode();
		return result;
	}
}
