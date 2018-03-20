

package com.badlogic.gdx.math;

import java.io.Serializable;

import com.badlogic.gdx.utils.NumberUtils;


public class Circle implements Serializable, Shape2D {
	public float x, y;
	public float radius;

	
	public Circle () {

	}

	
	public Circle (float x, float y, float radius) {
		this.x = x;
		this.y = y;
		this.radius = radius;
	}

	
	public Circle (Vector2 position, float radius) {
		this.x = position.x;
		this.y = position.y;
		this.radius = radius;
	}

	
	public Circle (Circle circle) {
		this.x = circle.x;
		this.y = circle.y;
		this.radius = circle.radius;
	}

	
	public Circle (Vector2 center, Vector2 edge) {
		this.x = center.x;
		this.y = center.y;
		this.radius = Vector2.len(center.x - edge.x, center.y - edge.y);
	}

	
	public void set (float x, float y, float radius) {
		this.x = x;
		this.y = y;
		this.radius = radius;
	}

	
	public void set (Vector2 position, float radius) {
		this.x = position.x;
		this.y = position.y;
		this.radius = radius;
	}

	
	public void set (Circle circle) {
		this.x = circle.x;
		this.y = circle.y;
		this.radius = circle.radius;
	}

	
	public void set (Vector2 center, Vector2 edge) {
		this.x = center.x;
		this.y = center.y;
		this.radius = Vector2.len(center.x - edge.x, center.y - edge.y);
	}

	
	public void setPosition (Vector2 position) {
		this.x = position.x;
		this.y = position.y;
	}

	
	public void setPosition (float x, float y) {
		this.x = x;
		this.y = y;
	}

	
	public void setX (float x) {
		this.x = x;
	}

	
	public void setY (float y) {
		this.y = y;
	}

	
	public void setRadius (float radius) {
		this.radius = radius;
	}

	
	public boolean contains (float x, float y) {
		x = this.x - x;
		y = this.y - y;
		return x * x + y * y <= radius * radius;
	}

	
	public boolean contains (Vector2 point) {
		float dx = x - point.x;
		float dy = y - point.y;
		return dx * dx + dy * dy <= radius * radius;
	}

	
	public boolean contains (Circle c) {
		final float radiusDiff = radius - c.radius;
		if (radiusDiff < 0f) return false; 		final float dx = x - c.x;
		final float dy = y - c.y;
		final float dst = dx * dx + dy * dy;
		final float radiusSum = radius + c.radius;
		return (!(radiusDiff * radiusDiff < dst) && (dst < radiusSum * radiusSum));
	}

	
	public boolean overlaps (Circle c) {
		float dx = x - c.x;
		float dy = y - c.y;
		float distance = dx * dx + dy * dy;
		float radiusSum = radius + c.radius;
		return distance < radiusSum * radiusSum;
	}

	
	@Override
	public String toString () {
		return x + "," + y + "," + radius;
	}

	
	public float circumference () {
		return this.radius * MathUtils.PI2;
	}

	
	public float area () {
		return this.radius * this.radius * MathUtils.PI;
	}

	@Override
	public boolean equals (Object o) {
		if (o == this) return true;
		if (o == null || o.getClass() != this.getClass()) return false;
		Circle c = (Circle)o;
		return this.x == c.x && this.y == c.y && this.radius == c.radius;
	}

	@Override
	public int hashCode () {
		final int prime = 41;
		int result = 1;
		result = prime * result + NumberUtils.floatToRawIntBits(radius);
		result = prime * result + NumberUtils.floatToRawIntBits(x);
		result = prime * result + NumberUtils.floatToRawIntBits(y);
		return result;
	}
}
