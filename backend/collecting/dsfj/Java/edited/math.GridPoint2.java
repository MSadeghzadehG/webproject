

package com.badlogic.gdx.math;

import java.io.Serializable;


public class GridPoint2 implements Serializable {
	private static final long serialVersionUID = -4019969926331717380L;

	public int x;
	public int y;

	
	public GridPoint2 () {
	}

	
	public GridPoint2 (int x, int y) {
		this.x = x;
		this.y = y;
	}

	
	public GridPoint2 (GridPoint2 point) {
		this.x = point.x;
		this.y = point.y;
	}

	
	public GridPoint2 set (GridPoint2 point) {
		this.x = point.x;
		this.y = point.y;
		return this;
	}

	
	public GridPoint2 set (int x, int y) {
		this.x = x;
		this.y = y;
		return this;
	}

	
	public float dst2 (GridPoint2 other) {
		int xd = other.x - x;
		int yd = other.y - y;

		return xd * xd + yd * yd;
	}

	
	public float dst2 (int x, int y) {
		int xd = x - this.x;
		int yd = y - this.y;

		return xd * xd + yd * yd;
	}

	
	public float dst (GridPoint2 other) {
		int xd = other.x - x;
		int yd = other.y - y;

		return (float)Math.sqrt(xd * xd + yd * yd);
	}

	
	public float dst (int x, int y) {
		int xd = x - this.x;
		int yd = y - this.y;

		return (float)Math.sqrt(xd * xd + yd * yd);
	}

	
	public GridPoint2 add (GridPoint2 other) {
		x += other.x;
		y += other.y;
		return this;
	}

	
	public GridPoint2 add (int x, int y) {
		this.x += x;
		this.y += y;
		return this;
	}

	
	public GridPoint2 sub (GridPoint2 other) {
		x -= other.x;
		y -= other.y;
		return this;
	}

	
	public GridPoint2 sub (int x, int y) {
		this.x -= x;
		this.y -= y;
		return this;
	}

	
	public GridPoint2 cpy () {
		return new GridPoint2(this);
	}

	@Override
	public boolean equals (Object o) {
		if (this == o) return true;
		if (o == null || o.getClass() != this.getClass()) return false;
		GridPoint2 g = (GridPoint2)o;
		return this.x == g.x && this.y == g.y;
	}

	@Override
	public int hashCode () {
		final int prime = 53;
		int result = 1;
		result = prime * result + this.x;
		result = prime * result + this.y;
		return result;
	}

	@Override
	public String toString () {
		return "(" + x + ", " + y + ")";
	}
}
