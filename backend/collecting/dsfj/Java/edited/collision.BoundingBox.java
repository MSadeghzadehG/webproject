

package com.badlogic.gdx.math.collision;

import java.io.Serializable;
import java.util.List;

import com.badlogic.gdx.math.Matrix4;
import com.badlogic.gdx.math.Vector3;


public class BoundingBox implements Serializable {
	private static final long serialVersionUID = -1286036817192127343L;

	private final static Vector3 tmpVector = new Vector3();

	public final Vector3 min = new Vector3();
	public final Vector3 max = new Vector3();

	private final Vector3 cnt = new Vector3();
	private final Vector3 dim = new Vector3();

	
	public Vector3 getCenter (Vector3 out) {
		return out.set(cnt);
	}

	public float getCenterX () {
		return cnt.x;
	}

	public float getCenterY () {
		return cnt.y;
	}

	public float getCenterZ () {
		return cnt.z;
	}

	public Vector3 getCorner000 (final Vector3 out) {
		return out.set(min.x, min.y, min.z);
	}

	public Vector3 getCorner001 (final Vector3 out) {
		return out.set(min.x, min.y, max.z);
	}

	public Vector3 getCorner010 (final Vector3 out) {
		return out.set(min.x, max.y, min.z);
	}

	public Vector3 getCorner011 (final Vector3 out) {
		return out.set(min.x, max.y, max.z);
	}

	public Vector3 getCorner100 (final Vector3 out) {
		return out.set(max.x, min.y, min.z);
	}

	public Vector3 getCorner101 (final Vector3 out) {
		return out.set(max.x, min.y, max.z);
	}

	public Vector3 getCorner110 (final Vector3 out) {
		return out.set(max.x, max.y, min.z);
	}

	public Vector3 getCorner111 (final Vector3 out) {
		return out.set(max.x, max.y, max.z);
	}

	
	public Vector3 getDimensions (final Vector3 out) {
		return out.set(dim);
	}

	public float getWidth () {
		return dim.x;
	}

	public float getHeight () {
		return dim.y;
	}

	public float getDepth () {
		return dim.z;
	}

	
	public Vector3 getMin (final Vector3 out) {
		return out.set(min);
	}

	
	public Vector3 getMax (final Vector3 out) {
		return out.set(max);
	}

	
	public BoundingBox () {
		clr();
	}

	
	public BoundingBox (BoundingBox bounds) {
		this.set(bounds);
	}

	
	public BoundingBox (Vector3 minimum, Vector3 maximum) {
		this.set(minimum, maximum);
	}

	
	public BoundingBox set (BoundingBox bounds) {
		return this.set(bounds.min, bounds.max);
	}

	
	public BoundingBox set (Vector3 minimum, Vector3 maximum) {
		min.set(minimum.x < maximum.x ? minimum.x : maximum.x, minimum.y < maximum.y ? minimum.y : maximum.y,
			minimum.z < maximum.z ? minimum.z : maximum.z);
		max.set(minimum.x > maximum.x ? minimum.x : maximum.x, minimum.y > maximum.y ? minimum.y : maximum.y,
			minimum.z > maximum.z ? minimum.z : maximum.z);
		cnt.set(min).add(max).scl(0.5f);
		dim.set(max).sub(min);
		return this;
	}

	
	public BoundingBox set (Vector3[] points) {
		this.inf();
		for (Vector3 l_point : points)
			this.ext(l_point);
		return this;
	}

	
	public BoundingBox set (List<Vector3> points) {
		this.inf();
		for (Vector3 l_point : points)
			this.ext(l_point);
		return this;
	}

	
	public BoundingBox inf () {
		min.set(Float.POSITIVE_INFINITY, Float.POSITIVE_INFINITY, Float.POSITIVE_INFINITY);
		max.set(Float.NEGATIVE_INFINITY, Float.NEGATIVE_INFINITY, Float.NEGATIVE_INFINITY);
		cnt.set(0, 0, 0);
		dim.set(0, 0, 0);
		return this;
	}

	
	public BoundingBox ext (Vector3 point) {
		return this.set(min.set(min(min.x, point.x), min(min.y, point.y), min(min.z, point.z)),
			max.set(Math.max(max.x, point.x), Math.max(max.y, point.y), Math.max(max.z, point.z)));
	}

	
	public BoundingBox clr () {
		return this.set(min.set(0, 0, 0), max.set(0, 0, 0));
	}

	
	public boolean isValid () {
		return min.x <= max.x && min.y <= max.y && min.z <= max.z;
	}

	
	public BoundingBox ext (BoundingBox a_bounds) {
		return this.set(min.set(min(min.x, a_bounds.min.x), min(min.y, a_bounds.min.y), min(min.z, a_bounds.min.z)),
			max.set(max(max.x, a_bounds.max.x), max(max.y, a_bounds.max.y), max(max.z, a_bounds.max.z)));
	}

	
	public BoundingBox ext (Vector3 center, float radius) {
		return this.set(min.set(min(min.x, center.x - radius), min(min.y, center.y - radius), min(min.z, center.z - radius)),
			max.set(max(max.x, center.x + radius), max(max.y, center.y + radius), max(max.z, center.z + radius)));
	}

	
	public BoundingBox ext (BoundingBox bounds, Matrix4 transform) {
		ext(tmpVector.set(bounds.min.x, bounds.min.y, bounds.min.z).mul(transform));
		ext(tmpVector.set(bounds.min.x, bounds.min.y, bounds.max.z).mul(transform));
		ext(tmpVector.set(bounds.min.x, bounds.max.y, bounds.min.z).mul(transform));
		ext(tmpVector.set(bounds.min.x, bounds.max.y, bounds.max.z).mul(transform));
		ext(tmpVector.set(bounds.max.x, bounds.min.y, bounds.min.z).mul(transform));
		ext(tmpVector.set(bounds.max.x, bounds.min.y, bounds.max.z).mul(transform));
		ext(tmpVector.set(bounds.max.x, bounds.max.y, bounds.min.z).mul(transform));
		ext(tmpVector.set(bounds.max.x, bounds.max.y, bounds.max.z).mul(transform));
		return this;
	}

	
	public BoundingBox mul (Matrix4 transform) {
		final float x0 = min.x, y0 = min.y, z0 = min.z, x1 = max.x, y1 = max.y, z1 = max.z;
		inf();
		ext(tmpVector.set(x0, y0, z0).mul(transform));
		ext(tmpVector.set(x0, y0, z1).mul(transform));
		ext(tmpVector.set(x0, y1, z0).mul(transform));
		ext(tmpVector.set(x0, y1, z1).mul(transform));
		ext(tmpVector.set(x1, y0, z0).mul(transform));
		ext(tmpVector.set(x1, y0, z1).mul(transform));
		ext(tmpVector.set(x1, y1, z0).mul(transform));
		ext(tmpVector.set(x1, y1, z1).mul(transform));
		return this;
	}

	
	public boolean contains (BoundingBox b) {
		return !isValid()
			|| (min.x <= b.min.x && min.y <= b.min.y && min.z <= b.min.z && max.x >= b.max.x && max.y >= b.max.y && max.z >= b.max.z);
	}

	
	public boolean intersects (BoundingBox b) {
		if (!isValid()) return false;

		
		float lx = Math.abs(this.cnt.x - b.cnt.x);
		float sumx = (this.dim.x / 2.0f) + (b.dim.x / 2.0f);

		float ly = Math.abs(this.cnt.y - b.cnt.y);
		float sumy = (this.dim.y / 2.0f) + (b.dim.y / 2.0f);

		float lz = Math.abs(this.cnt.z - b.cnt.z);
		float sumz = (this.dim.z / 2.0f) + (b.dim.z / 2.0f);

		return (lx <= sumx && ly <= sumy && lz <= sumz);

	}

	
	public boolean contains (Vector3 v) {
		return min.x <= v.x && max.x >= v.x && min.y <= v.y && max.y >= v.y && min.z <= v.z && max.z >= v.z;
	}

	@Override
	public String toString () {
		return "[" + min + "|" + max + "]";
	}

	
	public BoundingBox ext (float x, float y, float z) {
		return this.set(min.set(min(min.x, x), min(min.y, y), min(min.z, z)), max.set(max(max.x, x), max(max.y, y), max(max.z, z)));
	}

	static final float min (final float a, final float b) {
		return a > b ? b : a;
	}

	static final float max (final float a, final float b) {
		return a > b ? a : b;
	}
}
