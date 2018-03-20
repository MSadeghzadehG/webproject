

package com.badlogic.gdx.math.collision;

import java.io.Serializable;

import com.badlogic.gdx.math.MathUtils;
import com.badlogic.gdx.math.Vector3;
import com.badlogic.gdx.utils.NumberUtils;


public class Sphere implements Serializable {
	private static final long serialVersionUID = -6487336868908521596L;
	
	public float radius;
	
	public final Vector3 center;

	private static final float PI_4_3 = MathUtils.PI * 4f / 3f;

	
	public Sphere (Vector3 center, float radius) {
		this.center = new Vector3(center);
		this.radius = radius;
	}

	
	public boolean overlaps (Sphere sphere) {
		return center.dst2(sphere.center) < (radius + sphere.radius) * (radius + sphere.radius);
	}

	@Override
	public int hashCode () {
		final int prime = 71;
		int result = 1;
		result = prime * result + this.center.hashCode();
		result = prime * result + NumberUtils.floatToRawIntBits(this.radius);
		return result;
	}

	@Override
	public boolean equals (Object o) {
		if (this == o) return true;
		if (o == null || o.getClass() != this.getClass()) return false;
		Sphere s = (Sphere)o;
		return this.radius == s.radius && this.center.equals(s.center);
	}

	public float volume () {
		return PI_4_3 * this.radius * this.radius * this.radius;
	}

	public float surfaceArea () {
		return 4 * MathUtils.PI * this.radius * this.radius;
	}
}
