

package com.badlogic.gdx.math.collision;

import java.io.Serializable;

import com.badlogic.gdx.math.Vector3;
import com.badlogic.gdx.utils.NumberUtils;


public class Segment implements Serializable {
	private static final long serialVersionUID = 2739667069736519602L;

	
	public final Vector3 a = new Vector3();

	
	public final Vector3 b = new Vector3();

	
	public Segment (Vector3 a, Vector3 b) {
		this.a.set(a);
		this.b.set(b);
	}

	
	public Segment (float aX, float aY, float aZ, float bX, float bY, float bZ) {
		this.a.set(aX, aY, aZ);
		this.b.set(bX, bY, bZ);
	}

	public float len () {
		return a.dst(b);
	}

	public float len2 () {
		return a.dst2(b);
	}

	@Override
	public boolean equals (Object o) {
		if (o == this) return true;
		if (o == null || o.getClass() != this.getClass()) return false;
		Segment s = (Segment)o;
		return this.a.equals(s.a) && this.b.equals(s.b);
	}

	@Override
	public int hashCode () {
		final int prime = 71;
		int result = 1;
		result = prime * result + this.a.hashCode();
		result = prime * result + this.b.hashCode();
		return result;
	}
}
