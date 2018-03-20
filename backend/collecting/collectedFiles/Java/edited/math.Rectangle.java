

package com.badlogic.gdx.math;

import java.io.Serializable;

import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.NumberUtils;
import com.badlogic.gdx.utils.Scaling;


public class Rectangle implements Serializable, Shape2D {
	
	static public final Rectangle tmp = new Rectangle();

	
	static public final Rectangle tmp2 = new Rectangle();

	private static final long serialVersionUID = 5733252015138115702L;
	public float x, y;
	public float width, height;

	
	public Rectangle () {

	}

	
	public Rectangle (float x, float y, float width, float height) {
		this.x = x;
		this.y = y;
		this.width = width;
		this.height = height;
	}

	
	public Rectangle (Rectangle rect) {
		x = rect.x;
		y = rect.y;
		width = rect.width;
		height = rect.height;
	}

	
	public Rectangle set (float x, float y, float width, float height) {
		this.x = x;
		this.y = y;
		this.width = width;
		this.height = height;

		return this;
	}

	
	public float getX () {
		return x;
	}

	
	public Rectangle setX (float x) {
		this.x = x;

		return this;
	}

	
	public float getY () {
		return y;
	}

	
	public Rectangle setY (float y) {
		this.y = y;

		return this;
	}

	
	public float getWidth () {
		return width;
	}

	
	public Rectangle setWidth (float width) {
		this.width = width;

		return this;
	}

	
	public float getHeight () {
		return height;
	}

	
	public Rectangle setHeight (float height) {
		this.height = height;

		return this;
	}

	
	public Vector2 getPosition (Vector2 position) {
		return position.set(x, y);
	}

	
	public Rectangle setPosition (Vector2 position) {
		this.x = position.x;
		this.y = position.y;

		return this;
	}

	
	public Rectangle setPosition (float x, float y) {
		this.x = x;
		this.y = y;

		return this;
	}

	
	public Rectangle setSize (float width, float height) {
		this.width = width;
		this.height = height;

		return this;
	}

	
	public Rectangle setSize (float sizeXY) {
		this.width = sizeXY;
		this.height = sizeXY;

		return this;
	}

	
	public Vector2 getSize (Vector2 size) {
		return size.set(width, height);
	}

	
	public boolean contains (float x, float y) {
		return this.x <= x && this.x + this.width >= x && this.y <= y && this.y + this.height >= y;
	}

	
	public boolean contains (Vector2 point) {
		return contains(point.x, point.y);
	}

	
	public boolean contains (Circle circle) {
		return (circle.x - circle.radius >= x) && (circle.x + circle.radius <= x + width)
			&& (circle.y - circle.radius >= y) && (circle.y + circle.radius <= y + height);
	}

	
	public boolean contains (Rectangle rectangle) {
		float xmin = rectangle.x;
		float xmax = xmin + rectangle.width;

		float ymin = rectangle.y;
		float ymax = ymin + rectangle.height;

		return ((xmin > x && xmin < x + width) && (xmax > x && xmax < x + width))
			&& ((ymin > y && ymin < y + height) && (ymax > y && ymax < y + height));
	}

	
	public boolean overlaps (Rectangle r) {
		return x < r.x + r.width && x + width > r.x && y < r.y + r.height && y + height > r.y;
	}

	
	public Rectangle set (Rectangle rect) {
		this.x = rect.x;
		this.y = rect.y;
		this.width = rect.width;
		this.height = rect.height;

		return this;
	}

	
	public Rectangle merge (Rectangle rect) {
		float minX = Math.min(x, rect.x);
		float maxX = Math.max(x + width, rect.x + rect.width);
		x = minX;
		width = maxX - minX;

		float minY = Math.min(y, rect.y);
		float maxY = Math.max(y + height, rect.y + rect.height);
		y = minY;
		height = maxY - minY;

		return this;
	}

	
	public Rectangle merge (float x, float y) {
		float minX = Math.min(this.x, x);
		float maxX = Math.max(this.x + width, x);
		this.x = minX;
		this.width = maxX - minX;

		float minY = Math.min(this.y, y);
		float maxY = Math.max(this.y + height, y);
		this.y = minY;
		this.height = maxY - minY;

		return this;
	}

	
	public Rectangle merge (Vector2 vec) {
		return merge(vec.x, vec.y);
	}

	
	public Rectangle merge (Vector2[] vecs) {
		float minX = x;
		float maxX = x + width;
		float minY = y;
		float maxY = y + height;
		for (int i = 0; i < vecs.length; ++i) {
			Vector2 v = vecs[i];
			minX = Math.min(minX, v.x);
			maxX = Math.max(maxX, v.x);
			minY = Math.min(minY, v.y);
			maxY = Math.max(maxY, v.y);
		}
		x = minX;
		width = maxX - minX;
		y = minY;
		height = maxY - minY;
		return this;
	}

	
	public float getAspectRatio () {
		return (height == 0) ? Float.NaN : width / height;
	}

	
	public Vector2 getCenter (Vector2 vector) {
		vector.x = x + width / 2;
		vector.y = y + height / 2;
		return vector;
	}

	
	public Rectangle setCenter (float x, float y) {
		setPosition(x - width / 2, y - height / 2);
		return this;
	}

	
	public Rectangle setCenter (Vector2 position) {
		setPosition(position.x - width / 2, position.y - height / 2);
		return this;
	}

	
	public Rectangle fitOutside (Rectangle rect) {
		float ratio = getAspectRatio();

		if (ratio > rect.getAspectRatio()) {
						setSize(rect.height * ratio, rect.height);
		} else {
						setSize(rect.width, rect.width / ratio);
		}

		setPosition((rect.x + rect.width / 2) - width / 2, (rect.y + rect.height / 2) - height / 2);
		return this;
	}

	
	public Rectangle fitInside (Rectangle rect) {
		float ratio = getAspectRatio();

		if (ratio < rect.getAspectRatio()) {
						setSize(rect.height * ratio, rect.height);
		} else {
						setSize(rect.width, rect.width / ratio);
		}

		setPosition((rect.x + rect.width / 2) - width / 2, (rect.y + rect.height / 2) - height / 2);
		return this;
	}

	
	public String toString () {
		return "[" + x + "," + y + "," + width + "," + height + "]";
	}

	
	public Rectangle fromString (String v) {
		int s0 = v.indexOf(',', 1);
		int s1 = v.indexOf(',', s0 + 1);
		int s2 = v.indexOf(',', s1 + 1);
		if (s0 != -1 && s1 != -1 && s2 != -1 && v.charAt(0) == '[' && v.charAt(v.length() - 1) == ']') {
			try {
				float x = Float.parseFloat(v.substring(1, s0));
				float y = Float.parseFloat(v.substring(s0 + 1, s1));
				float width = Float.parseFloat(v.substring(s1 + 1, s2));
				float height = Float.parseFloat(v.substring(s2 + 1, v.length() - 1));
				return this.set(x, y, width, height);
			} catch (NumberFormatException ex) {
							}
		}
		throw new GdxRuntimeException("Malformed Rectangle: " + v);
	}

	public float area () {
		return this.width * this.height;
	}

	public float perimeter () {
		return 2 * (this.width + this.height);
	}

	public int hashCode () {
		final int prime = 31;
		int result = 1;
		result = prime * result + NumberUtils.floatToRawIntBits(height);
		result = prime * result + NumberUtils.floatToRawIntBits(width);
		result = prime * result + NumberUtils.floatToRawIntBits(x);
		result = prime * result + NumberUtils.floatToRawIntBits(y);
		return result;
	}

	public boolean equals (Object obj) {
		if (this == obj) return true;
		if (obj == null) return false;
		if (getClass() != obj.getClass()) return false;
		Rectangle other = (Rectangle)obj;
		if (NumberUtils.floatToRawIntBits(height) != NumberUtils.floatToRawIntBits(other.height)) return false;
		if (NumberUtils.floatToRawIntBits(width) != NumberUtils.floatToRawIntBits(other.width)) return false;
		if (NumberUtils.floatToRawIntBits(x) != NumberUtils.floatToRawIntBits(other.x)) return false;
		if (NumberUtils.floatToRawIntBits(y) != NumberUtils.floatToRawIntBits(other.y)) return false;
		return true;
	}

}
