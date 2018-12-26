

package com.badlogic.gdx.math;

import java.io.Serializable;

import com.badlogic.gdx.math.Vector2;
import com.badlogic.gdx.utils.NumberUtils;


public class Ellipse implements Serializable, Shape2D {

	public float x, y;
	public float width, height;

	private static final long serialVersionUID = 7381533206532032099L;

	
	public Ellipse () {

	}

	
	public Ellipse (Ellipse ellipse) {
		this.x = ellipse.x;
		this.y = ellipse.y;
		this.width = ellipse.width;
		this.height = ellipse.height;
	}

	
	public Ellipse (float x, float y, float width, float height) {
		this.x = x;
		this.y = y;
		this.width = width;
		this.height = height;
	}

	
	public Ellipse (Vector2 position, float width, float height) {
		this.x = position.x;
		this.y = position.y;
		this.width = width;
		this.height = height;
	}

	public Ellipse (Vector2 position, Vector2 size) {
		this.x = position.x;
		this.y = position.y;
		this.width = size.x;
		this.height = size.y;
	}

	
	public Ellipse (Circle circle) {
		this.x = circle.x;
		this.y = circle.y;
		this.width = circle.radius;
		this.height = circle.radius;
	}

	
	public boolean contains (float x, float y) {
		x = x - this.x;
		y = y - this.y;

		return (x * x) / (width * 0.5f * width * 0.5f) + (y * y) / (height * 0.5f * height * 0.5f) <= 1.0f;
	}

	
	public boolean contains (Vector2 point) {
		return contains(point.x, point.y);
	}

	
	public void set (float x, float y, float width, float height) {
		this.x = x;
		this.y = y;
		this.width = width;
		this.height = height;
	}

	
	public void set (Ellipse ellipse) {
		x = ellipse.x;
		y = ellipse.y;
		width = ellipse.width;
		height = ellipse.height;
	}

	public void set (Circle circle) {
		this.x = circle.x;
		this.y = circle.y;
		this.width = circle.radius;
		this.height = circle.radius;
	}

	public void set (Vector2 position, Vector2 size) {
		this.x = position.x;
		this.y = position.y;
		this.width = size.x;
		this.height = size.y;
	}

	
	public Ellipse setPosition (Vector2 position) {
		this.x = position.x;
		this.y = position.y;

		return this;
	}

	
	public Ellipse setPosition (float x, float y) {
		this.x = x;
		this.y = y;

		return this;
	}

	
	public Ellipse setSize (float width, float height) {
		this.width = width;
		this.height = height;

		return this;
	}

	
	public float area () {
		return MathUtils.PI * (this.width * this.height) / 4;
	}

	
	public float circumference () {
		float a = this.width / 2;
		float b = this.height / 2;
		if (a * 3 > b || b * 3 > a) {
						return (float)(MathUtils.PI * ((3 * (a + b)) - Math.sqrt((3 * a + b) * (a + 3 * b))));
		} else {
						return (float)(MathUtils.PI2 * Math.sqrt((a * a + b * b) / 2));
		}
	}

	@Override
	public boolean equals (Object o) {
		if (o == this) return true;
		if (o == null || o.getClass() != this.getClass()) return false;
		Ellipse e = (Ellipse)o;
		return this.x == e.x && this.y == e.y && this.width == e.width && this.height == e.height;
	}

	@Override
	public int hashCode () {
		final int prime = 53;
		int result = 1;
		result = prime * result + NumberUtils.floatToRawIntBits(this.height);
		result = prime * result + NumberUtils.floatToRawIntBits(this.width);
		result = prime * result + NumberUtils.floatToRawIntBits(this.x);
		result = prime * result + NumberUtils.floatToRawIntBits(this.y);
		return result;
	}
}
