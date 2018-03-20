

package com.badlogic.gdx.math;

import java.io.Serializable;

import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.NumberUtils;


public class Vector3 implements Serializable, Vector<Vector3> {
	private static final long serialVersionUID = 3840054589595372522L;

	
	public float x;
	
	public float y;
	
	public float z;

	public final static Vector3 X = new Vector3(1, 0, 0);
	public final static Vector3 Y = new Vector3(0, 1, 0);
	public final static Vector3 Z = new Vector3(0, 0, 1);
	public final static Vector3 Zero = new Vector3(0, 0, 0);

	private final static Matrix4 tmpMat = new Matrix4();

	
	public Vector3 () {
	}

	
	public Vector3 (float x, float y, float z) {
		this.set(x, y, z);
	}

	
	public Vector3 (final Vector3 vector) {
		this.set(vector);
	}

	
	public Vector3 (final float[] values) {
		this.set(values[0], values[1], values[2]);
	}

	
	public Vector3 (final Vector2 vector, float z) {
		this.set(vector.x, vector.y, z);
	}

	
	public Vector3 set (float x, float y, float z) {
		this.x = x;
		this.y = y;
		this.z = z;
		return this;
	}

	@Override
	public Vector3 set (final Vector3 vector) {
		return this.set(vector.x, vector.y, vector.z);
	}

	
	public Vector3 set (final float[] values) {
		return this.set(values[0], values[1], values[2]);
	}

	
	public Vector3 set (final Vector2 vector, float z) {
		return this.set(vector.x, vector.y, z);
	}

	
	public Vector3 setFromSpherical (float azimuthalAngle, float polarAngle) {
		float cosPolar = MathUtils.cos(polarAngle);
		float sinPolar = MathUtils.sin(polarAngle);

		float cosAzim = MathUtils.cos(azimuthalAngle);
		float sinAzim = MathUtils.sin(azimuthalAngle);

		return this.set(cosAzim * sinPolar, sinAzim * sinPolar, cosPolar);
	}

	@Override
	public Vector3 setToRandomDirection () {
		float u = MathUtils.random();
		float v = MathUtils.random();

		float theta = MathUtils.PI2 * u; 		float phi = (float)Math.acos(2f * v - 1f); 
		return this.setFromSpherical(theta, phi);
	}

	@Override
	public Vector3 cpy () {
		return new Vector3(this);
	}

	@Override
	public Vector3 add (final Vector3 vector) {
		return this.add(vector.x, vector.y, vector.z);
	}

	
	public Vector3 add (float x, float y, float z) {
		return this.set(this.x + x, this.y + y, this.z + z);
	}

	
	public Vector3 add (float values) {
		return this.set(this.x + values, this.y + values, this.z + values);
	}

	@Override
	public Vector3 sub (final Vector3 a_vec) {
		return this.sub(a_vec.x, a_vec.y, a_vec.z);
	}

	
	public Vector3 sub (float x, float y, float z) {
		return this.set(this.x - x, this.y - y, this.z - z);
	}

	
	public Vector3 sub (float value) {
		return this.set(this.x - value, this.y - value, this.z - value);
	}

	@Override
	public Vector3 scl (float scalar) {
		return this.set(this.x * scalar, this.y * scalar, this.z * scalar);
	}

	@Override
	public Vector3 scl (final Vector3 other) {
		return this.set(x * other.x, y * other.y, z * other.z);
	}

	
	public Vector3 scl (float vx, float vy, float vz) {
		return this.set(this.x * vx, this.y * vy, this.z * vz);
	}

	@Override
	public Vector3 mulAdd (Vector3 vec, float scalar) {
		this.x += vec.x * scalar;
		this.y += vec.y * scalar;
		this.z += vec.z * scalar;
		return this;
	}

	@Override
	public Vector3 mulAdd (Vector3 vec, Vector3 mulVec) {
		this.x += vec.x * mulVec.x;
		this.y += vec.y * mulVec.y;
		this.z += vec.z * mulVec.z;
		return this;
	}

	
	public static float len (final float x, final float y, final float z) {
		return (float)Math.sqrt(x * x + y * y + z * z);
	}

	@Override
	public float len () {
		return (float)Math.sqrt(x * x + y * y + z * z);
	}

	
	public static float len2 (final float x, final float y, final float z) {
		return x * x + y * y + z * z;
	}

	@Override
	public float len2 () {
		return x * x + y * y + z * z;
	}

	
	public boolean idt (final Vector3 vector) {
		return x == vector.x && y == vector.y && z == vector.z;
	}

	
	public static float dst (final float x1, final float y1, final float z1, final float x2, final float y2, final float z2) {
		final float a = x2 - x1;
		final float b = y2 - y1;
		final float c = z2 - z1;
		return (float)Math.sqrt(a * a + b * b + c * c);
	}

	@Override
	public float dst (final Vector3 vector) {
		final float a = vector.x - x;
		final float b = vector.y - y;
		final float c = vector.z - z;
		return (float)Math.sqrt(a * a + b * b + c * c);
	}

	
	public float dst (float x, float y, float z) {
		final float a = x - this.x;
		final float b = y - this.y;
		final float c = z - this.z;
		return (float)Math.sqrt(a * a + b * b + c * c);
	}

	
	public static float dst2 (final float x1, final float y1, final float z1, final float x2, final float y2, final float z2) {
		final float a = x2 - x1;
		final float b = y2 - y1;
		final float c = z2 - z1;
		return a * a + b * b + c * c;
	}

	@Override
	public float dst2 (Vector3 point) {
		final float a = point.x - x;
		final float b = point.y - y;
		final float c = point.z - z;
		return a * a + b * b + c * c;
	}

	
	public float dst2 (float x, float y, float z) {
		final float a = x - this.x;
		final float b = y - this.y;
		final float c = z - this.z;
		return a * a + b * b + c * c;
	}

	@Override
	public Vector3 nor () {
		final float len2 = this.len2();
		if (len2 == 0f || len2 == 1f) return this;
		return this.scl(1f / (float)Math.sqrt(len2));
	}

	
	public static float dot (float x1, float y1, float z1, float x2, float y2, float z2) {
		return x1 * x2 + y1 * y2 + z1 * z2;
	}

	@Override
	public float dot (final Vector3 vector) {
		return x * vector.x + y * vector.y + z * vector.z;
	}

	
	public float dot (float x, float y, float z) {
		return this.x * x + this.y * y + this.z * z;
	}

	
	public Vector3 crs (final Vector3 vector) {
		return this.set(y * vector.z - z * vector.y, z * vector.x - x * vector.z, x * vector.y - y * vector.x);
	}

	
	public Vector3 crs (float x, float y, float z) {
		return this.set(this.y * z - this.z * y, this.z * x - this.x * z, this.x * y - this.y * x);
	}

	
	public Vector3 mul4x3 (float[] matrix) {
		return set(x * matrix[0] + y * matrix[3] + z * matrix[6] + matrix[9], x * matrix[1] + y * matrix[4] + z * matrix[7]
			+ matrix[10], x * matrix[2] + y * matrix[5] + z * matrix[8] + matrix[11]);
	}

	
	public Vector3 mul (final Matrix4 matrix) {
		final float l_mat[] = matrix.val;
		return this.set(x * l_mat[Matrix4.M00] + y * l_mat[Matrix4.M01] + z * l_mat[Matrix4.M02] + l_mat[Matrix4.M03], x
			* l_mat[Matrix4.M10] + y * l_mat[Matrix4.M11] + z * l_mat[Matrix4.M12] + l_mat[Matrix4.M13], x * l_mat[Matrix4.M20] + y
			* l_mat[Matrix4.M21] + z * l_mat[Matrix4.M22] + l_mat[Matrix4.M23]);
	}

	
	public Vector3 traMul (final Matrix4 matrix) {
		final float l_mat[] = matrix.val;
		return this.set(x * l_mat[Matrix4.M00] + y * l_mat[Matrix4.M10] + z * l_mat[Matrix4.M20] + l_mat[Matrix4.M30], x
			* l_mat[Matrix4.M01] + y * l_mat[Matrix4.M11] + z * l_mat[Matrix4.M21] + l_mat[Matrix4.M31], x * l_mat[Matrix4.M02] + y
			* l_mat[Matrix4.M12] + z * l_mat[Matrix4.M22] + l_mat[Matrix4.M32]);
	}

	
	public Vector3 mul (Matrix3 matrix) {
		final float l_mat[] = matrix.val;
		return set(x * l_mat[Matrix3.M00] + y * l_mat[Matrix3.M01] + z * l_mat[Matrix3.M02], x * l_mat[Matrix3.M10] + y
			* l_mat[Matrix3.M11] + z * l_mat[Matrix3.M12], x * l_mat[Matrix3.M20] + y * l_mat[Matrix3.M21] + z * l_mat[Matrix3.M22]);
	}

	
	public Vector3 traMul (Matrix3 matrix) {
		final float l_mat[] = matrix.val;
		return set(x * l_mat[Matrix3.M00] + y * l_mat[Matrix3.M10] + z * l_mat[Matrix3.M20], x * l_mat[Matrix3.M01] + y
			* l_mat[Matrix3.M11] + z * l_mat[Matrix3.M21], x * l_mat[Matrix3.M02] + y * l_mat[Matrix3.M12] + z * l_mat[Matrix3.M22]);
	}

	
	public Vector3 mul (final Quaternion quat) {
		return quat.transform(this);
	}

	
	public Vector3 prj (final Matrix4 matrix) {
		final float l_mat[] = matrix.val;
		final float l_w = 1f / (x * l_mat[Matrix4.M30] + y * l_mat[Matrix4.M31] + z * l_mat[Matrix4.M32] + l_mat[Matrix4.M33]);
		return this.set((x * l_mat[Matrix4.M00] + y * l_mat[Matrix4.M01] + z * l_mat[Matrix4.M02] + l_mat[Matrix4.M03]) * l_w, (x
			* l_mat[Matrix4.M10] + y * l_mat[Matrix4.M11] + z * l_mat[Matrix4.M12] + l_mat[Matrix4.M13])
			* l_w, (x * l_mat[Matrix4.M20] + y * l_mat[Matrix4.M21] + z * l_mat[Matrix4.M22] + l_mat[Matrix4.M23]) * l_w);
	}

	
	public Vector3 rot (final Matrix4 matrix) {
		final float l_mat[] = matrix.val;
		return this.set(x * l_mat[Matrix4.M00] + y * l_mat[Matrix4.M01] + z * l_mat[Matrix4.M02], x * l_mat[Matrix4.M10] + y
			* l_mat[Matrix4.M11] + z * l_mat[Matrix4.M12], x * l_mat[Matrix4.M20] + y * l_mat[Matrix4.M21] + z * l_mat[Matrix4.M22]);
	}

	
	public Vector3 unrotate (final Matrix4 matrix) {
		final float l_mat[] = matrix.val;
		return this.set(x * l_mat[Matrix4.M00] + y * l_mat[Matrix4.M10] + z * l_mat[Matrix4.M20], x * l_mat[Matrix4.M01] + y
			* l_mat[Matrix4.M11] + z * l_mat[Matrix4.M21], x * l_mat[Matrix4.M02] + y * l_mat[Matrix4.M12] + z * l_mat[Matrix4.M22]);
	}

	
	public Vector3 untransform (final Matrix4 matrix) {
		final float l_mat[] = matrix.val;
		x -= l_mat[Matrix4.M03];
		y -= l_mat[Matrix4.M03];
		z -= l_mat[Matrix4.M03];
		return this.set(x * l_mat[Matrix4.M00] + y * l_mat[Matrix4.M10] + z * l_mat[Matrix4.M20], x * l_mat[Matrix4.M01] + y
			* l_mat[Matrix4.M11] + z * l_mat[Matrix4.M21], x * l_mat[Matrix4.M02] + y * l_mat[Matrix4.M12] + z * l_mat[Matrix4.M22]);
	}

	
	public Vector3 rotate (float degrees, float axisX, float axisY, float axisZ) {
		return this.mul(tmpMat.setToRotation(axisX, axisY, axisZ, degrees));
	}

	
	public Vector3 rotateRad (float radians, float axisX, float axisY, float axisZ) {
		return this.mul(tmpMat.setToRotationRad(axisX, axisY, axisZ, radians));
	}

	
	public Vector3 rotate (final Vector3 axis, float degrees) {
		tmpMat.setToRotation(axis, degrees);
		return this.mul(tmpMat);
	}

	
	public Vector3 rotateRad (final Vector3 axis, float radians) {
		tmpMat.setToRotationRad(axis, radians);
		return this.mul(tmpMat);
	}

	@Override
	public boolean isUnit () {
		return isUnit(0.000000001f);
	}

	@Override
	public boolean isUnit (final float margin) {
		return Math.abs(len2() - 1f) < margin;
	}

	@Override
	public boolean isZero () {
		return x == 0 && y == 0 && z == 0;
	}

	@Override
	public boolean isZero (final float margin) {
		return len2() < margin;
	}

	@Override
	public boolean isOnLine (Vector3 other, float epsilon) {
		return len2(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x) <= epsilon;
	}

	@Override
	public boolean isOnLine (Vector3 other) {
		return len2(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x) <= MathUtils.FLOAT_ROUNDING_ERROR;
	}

	@Override
	public boolean isCollinear (Vector3 other, float epsilon) {
		return isOnLine(other, epsilon) && hasSameDirection(other);
	}

	@Override
	public boolean isCollinear (Vector3 other) {
		return isOnLine(other) && hasSameDirection(other);
	}

	@Override
	public boolean isCollinearOpposite (Vector3 other, float epsilon) {
		return isOnLine(other, epsilon) && hasOppositeDirection(other);
	}

	@Override
	public boolean isCollinearOpposite (Vector3 other) {
		return isOnLine(other) && hasOppositeDirection(other);
	}

	@Override
	public boolean isPerpendicular (Vector3 vector) {
		return MathUtils.isZero(dot(vector));
	}

	@Override
	public boolean isPerpendicular (Vector3 vector, float epsilon) {
		return MathUtils.isZero(dot(vector), epsilon);
	}

	@Override
	public boolean hasSameDirection (Vector3 vector) {
		return dot(vector) > 0;
	}

	@Override
	public boolean hasOppositeDirection (Vector3 vector) {
		return dot(vector) < 0;
	}

	@Override
	public Vector3 lerp (final Vector3 target, float alpha) {
		x += alpha * (target.x - x);
		y += alpha * (target.y - y);
		z += alpha * (target.z - z);
		return this;
	}

	@Override
	public Vector3 interpolate (Vector3 target, float alpha, Interpolation interpolator) {
		return lerp(target, interpolator.apply(0f, 1f, alpha));
	}

	
	public Vector3 slerp (final Vector3 target, float alpha) {
		final float dot = dot(target);
				if (dot > 0.9995 || dot < -0.9995) return lerp(target, alpha);

				final float theta0 = (float)Math.acos(dot);
				final float theta = theta0 * alpha;

		final float st = (float)Math.sin(theta);
		final float tx = target.x - x * dot;
		final float ty = target.y - y * dot;
		final float tz = target.z - z * dot;
		final float l2 = tx * tx + ty * ty + tz * tz;
		final float dl = st * ((l2 < 0.0001f) ? 1f : 1f / (float)Math.sqrt(l2));

		return scl((float)Math.cos(theta)).add(tx * dl, ty * dl, tz * dl).nor();
	}

	
	@Override
	public String toString () {
		return "(" + x + "," + y + "," + z + ")";
	}

	
	public Vector3 fromString (String v) {
		int s0 = v.indexOf(',', 1);
		int s1 = v.indexOf(',', s0 + 1);
		if (s0 != -1 && s1 != -1 && v.charAt(0) == '(' && v.charAt(v.length() - 1) == ')') {
			try {
				float x = Float.parseFloat(v.substring(1, s0));
				float y = Float.parseFloat(v.substring(s0 + 1, s1));
				float z = Float.parseFloat(v.substring(s1 + 1, v.length() - 1));
				return this.set(x, y, z);
			} catch (NumberFormatException ex) {
							}
		}
		throw new GdxRuntimeException("Malformed Vector3: " + v);
	}

	@Override
	public Vector3 limit (float limit) {
		return limit2(limit * limit);
	}

	@Override
	public Vector3 limit2 (float limit2) {
		float len2 = len2();
		if (len2 > limit2) {
			scl((float)Math.sqrt(limit2 / len2));
		}
		return this;
	}

	@Override
	public Vector3 setLength (float len) {
		return setLength2(len * len);
	}

	@Override
	public Vector3 setLength2 (float len2) {
		float oldLen2 = len2();
		return (oldLen2 == 0 || oldLen2 == len2) ? this : scl((float)Math.sqrt(len2 / oldLen2));
	}

	@Override
	public Vector3 clamp (float min, float max) {
		final float len2 = len2();
		if (len2 == 0f) return this;
		float max2 = max * max;
		if (len2 > max2) return scl((float)Math.sqrt(max2 / len2));
		float min2 = min * min;
		if (len2 < min2) return scl((float)Math.sqrt(min2 / len2));
		return this;
	}

	@Override
	public int hashCode () {
		final int prime = 31;
		int result = 1;
		result = prime * result + NumberUtils.floatToIntBits(x);
		result = prime * result + NumberUtils.floatToIntBits(y);
		result = prime * result + NumberUtils.floatToIntBits(z);
		return result;
	}

	@Override
	public boolean equals (Object obj) {
		if (this == obj) return true;
		if (obj == null) return false;
		if (getClass() != obj.getClass()) return false;
		Vector3 other = (Vector3)obj;
		if (NumberUtils.floatToIntBits(x) != NumberUtils.floatToIntBits(other.x)) return false;
		if (NumberUtils.floatToIntBits(y) != NumberUtils.floatToIntBits(other.y)) return false;
		if (NumberUtils.floatToIntBits(z) != NumberUtils.floatToIntBits(other.z)) return false;
		return true;
	}

	@Override
	public boolean epsilonEquals (final Vector3 other, float epsilon) {
		if (other == null) return false;
		if (Math.abs(other.x - x) > epsilon) return false;
		if (Math.abs(other.y - y) > epsilon) return false;
		if (Math.abs(other.z - z) > epsilon) return false;
		return true;
	}

	
	public boolean epsilonEquals (float x, float y, float z, float epsilon) {
		if (Math.abs(x - this.x) > epsilon) return false;
		if (Math.abs(y - this.y) > epsilon) return false;
		if (Math.abs(z - this.z) > epsilon) return false;
		return true;
	}

	
	public boolean epsilonEquals (final Vector3 other) {
		return epsilonEquals(other, MathUtils.FLOAT_ROUNDING_ERROR);
	}

	
	public boolean epsilonEquals (float x, float y, float z) {
		return epsilonEquals(x, y, z, MathUtils.FLOAT_ROUNDING_ERROR);
	}

	@Override
	public Vector3 setZero () {
		this.x = 0;
		this.y = 0;
		this.z = 0;
		return this;
	}
}
