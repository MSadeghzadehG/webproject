

package com.badlogic.gdx.math;

import java.io.Serializable;


public class GridPoint3 implements Serializable {
	private static final long serialVersionUID = 5922187982746752830L;

	public int x;
	public int y;
	public int z;

	
	public GridPoint3 () {
	}

	
	public GridPoint3 (int x, int y, int z) {
		this.x = x;
		this.y = y;
		this.z = z;
	}

	
	public GridPoint3 (GridPoint3 point) {
		this.x = point.x;
		this.y = point.y;
		this.z = point.z;
	}

	
	public GridPoint3 set (GridPoint3 point) {
		this.x = point.x;
		this.y = point.y;
		this.z = point.z;
		return this;
	}

	
	public GridPoint3 set (int x, int y, int z) {
		this.x = x;
		this.y = y;
		this.z = z;
		return this;
	}

	
	public float dst2 (GridPoint3 other) {
		int xd = other.x - x;
		int yd = other.y - y;
		int zd = other.z - z;

		return xd * xd + yd * yd + zd * zd;
	}

	
	public float dst2 (int x, int y, int z) {
		int xd = x - this.x;
		int yd = y - this.y;
		int zd = z - this.z;

		return xd * xd + yd * yd + zd * zd;
	}

	
	public float dst (GridPoint3 other) {
		int xd = other.x - x;
		int yd = other.y - y;
		int zd = other.z - z;

		return (float)Math.sqrt(xd * xd + yd * yd + zd * zd);
	}

	
	public float dst (int x, int y, int z) {
		int xd = x - this.x;
		int yd = y - this.y;
		int zd = z - this.z;

		return (float)Math.sqrt(xd * xd + yd * yd + zd * zd);
	}

	
	public GridPoint3 add (GridPoint3 other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return this;
	}

	
	public GridPoint3 add (int x, int y, int z) {
		this.x += x;
		this.y += y;
		this.z += z;
		return this;
	}

	
	public GridPoint3 sub (GridPoint3 other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return this;
	}

	
	public GridPoint3 sub (int x, int y, int z) {
		this.x -= x;
		this.y -= y;
		this.z -= z;
		return this;
	}

	
	public GridPoint3 cpy () {
		return new GridPoint3(this);
	}

	@Override
	public boolean equals (Object o) {
		if (this == o) return true;
		if (o == null || o.getClass() != this.getClass()) return false;
		GridPoint3 g = (GridPoint3)o;
		return this.x == g.x && this.y == g.y && this.z == g.z;
	}

	@Override
	public int hashCode () {
		final int prime = 17;
		int result = 1;
		result = prime * result + this.x;
		result = prime * result + this.y;
		result = prime * result + this.z;
		return result;
	}

	@Override
	public String toString () {
		return "(" + x + ", " + y + ", " + z + ")";
	}
}
