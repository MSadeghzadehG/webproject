

package com.badlogic.gdx.math;

import java.io.Serializable;

import com.badlogic.gdx.utils.GdxRuntimeException;


public final class Affine2 implements Serializable {
	private static final long serialVersionUID = 1524569123485049187L;

	public float m00 = 1, m01 = 0, m02 = 0;
	public float m10 = 0, m11 = 1, m12 = 0;

	
	
	public Affine2 () {
	}

	
	public Affine2 (Affine2 other) {
		set(other);
	}

	
	public Affine2 idt () {
		m00 = 1;
		m01 = 0;
		m02 = 0;
		m10 = 0;
		m11 = 1;
		m12 = 0;
		return this;
	}

	
	public Affine2 set (Affine2 other) {
		m00 = other.m00;
		m01 = other.m01;
		m02 = other.m02;
		m10 = other.m10;
		m11 = other.m11;
		m12 = other.m12;
		return this;
	}

	
	public Affine2 set (Matrix3 matrix) {
		float[] other = matrix.val;

		m00 = other[Matrix3.M00];
		m01 = other[Matrix3.M01];
		m02 = other[Matrix3.M02];
		m10 = other[Matrix3.M10];
		m11 = other[Matrix3.M11];
		m12 = other[Matrix3.M12];
		return this;
	}

	
	public Affine2 set (Matrix4 matrix) {
		float[] other = matrix.val;

		m00 = other[Matrix4.M00];
		m01 = other[Matrix4.M01];
		m02 = other[Matrix4.M03];
		m10 = other[Matrix4.M10];
		m11 = other[Matrix4.M11];
		m12 = other[Matrix4.M13];
		return this;
	}

	
	public Affine2 setToTranslation (float x, float y) {
		m00 = 1;
		m01 = 0;
		m02 = x;
		m10 = 0;
		m11 = 1;
		m12 = y;
		return this;
	}

	
	public Affine2 setToTranslation (Vector2 trn) {
		return setToTranslation(trn.x, trn.y);
	}

	
	public Affine2 setToScaling (float scaleX, float scaleY) {
		m00 = scaleX;
		m01 = 0;
		m02 = 0;
		m10 = 0;
		m11 = scaleY;
		m12 = 0;
		return this;
	}

	
	public Affine2 setToScaling (Vector2 scale) {
		return setToScaling(scale.x, scale.y);
	}

	
	public Affine2 setToRotation (float degrees) {
		float cos = MathUtils.cosDeg(degrees);
		float sin = MathUtils.sinDeg(degrees);

		m00 = cos;
		m01 = -sin;
		m02 = 0;
		m10 = sin;
		m11 = cos;
		m12 = 0;
		return this;
	}

	
	public Affine2 setToRotationRad (float radians) {
		float cos = MathUtils.cos(radians);
		float sin = MathUtils.sin(radians);

		m00 = cos;
		m01 = -sin;
		m02 = 0;
		m10 = sin;
		m11 = cos;
		m12 = 0;
		return this;
	}

	
	public Affine2 setToRotation (float cos, float sin) {
		m00 = cos;
		m01 = -sin;
		m02 = 0;
		m10 = sin;
		m11 = cos;
		m12 = 0;
		return this;
	}

	
	public Affine2 setToShearing (float shearX, float shearY) {
		m00 = 1;
		m01 = shearX;
		m02 = 0;
		m10 = shearY;
		m11 = 1;
		m12 = 0;
		return this;
	}

	
	public Affine2 setToShearing (Vector2 shear) {
		return setToShearing(shear.x, shear.y);
	}

	
	public Affine2 setToTrnRotScl (float x, float y, float degrees, float scaleX, float scaleY) {
		m02 = x;
		m12 = y;

		if (degrees == 0) {
			m00 = scaleX;
			m01 = 0;
			m10 = 0;
			m11 = scaleY;
		} else {
			float sin = MathUtils.sinDeg(degrees);
			float cos = MathUtils.cosDeg(degrees);

			m00 = cos * scaleX;
			m01 = -sin * scaleY;
			m10 = sin * scaleX;
			m11 = cos * scaleY;
		}
		return this;
	}

	
	public Affine2 setToTrnRotScl (Vector2 trn, float degrees, Vector2 scale) {
		return setToTrnRotScl(trn.x, trn.y, degrees, scale.x, scale.y);
	}

	
	public Affine2 setToTrnRotRadScl (float x, float y, float radians, float scaleX, float scaleY) {
		m02 = x;
		m12 = y;

		if (radians == 0) {
			m00 = scaleX;
			m01 = 0;
			m10 = 0;
			m11 = scaleY;
		} else {
			float sin = MathUtils.sin(radians);
			float cos = MathUtils.cos(radians);

			m00 = cos * scaleX;
			m01 = -sin * scaleY;
			m10 = sin * scaleX;
			m11 = cos * scaleY;
		}
		return this;
	}

	
	public Affine2 setToTrnRotRadScl (Vector2 trn, float radians, Vector2 scale) {
		return setToTrnRotRadScl(trn.x, trn.y, radians, scale.x, scale.y);
	}

	
	public Affine2 setToTrnScl (float x, float y, float scaleX, float scaleY) {
		m00 = scaleX;
		m01 = 0;
		m02 = x;
		m10 = 0;
		m11 = scaleY;
		m12 = y;
		return this;
	}

	
	public Affine2 setToTrnScl (Vector2 trn, Vector2 scale) {
		return setToTrnScl(trn.x, trn.y, scale.x, scale.y);
	}

	
	public Affine2 setToProduct (Affine2 l, Affine2 r) {
		m00 = l.m00 * r.m00 + l.m01 * r.m10;
		m01 = l.m00 * r.m01 + l.m01 * r.m11;
		m02 = l.m00 * r.m02 + l.m01 * r.m12 + l.m02;
		m10 = l.m10 * r.m00 + l.m11 * r.m10;
		m11 = l.m10 * r.m01 + l.m11 * r.m11;
		m12 = l.m10 * r.m02 + l.m11 * r.m12 + l.m12;
		return this;
	}

	
	public Affine2 inv () {
		float det = det();
		if (det == 0) throw new GdxRuntimeException("Can't invert a singular affine matrix");

		float invDet = 1.0f / det;

		float tmp00 = m11;
		float tmp01 = -m01;
		float tmp02 = m01 * m12 - m11 * m02;
		float tmp10 = -m10;
		float tmp11 = m00;
		float tmp12 = m10 * m02 - m00 * m12;

		m00 = invDet * tmp00;
		m01 = invDet * tmp01;
		m02 = invDet * tmp02;
		m10 = invDet * tmp10;
		m11 = invDet * tmp11;
		m12 = invDet * tmp12;
		return this;
	}

	
	public Affine2 mul (Affine2 other) {
		float tmp00 = m00 * other.m00 + m01 * other.m10;
		float tmp01 = m00 * other.m01 + m01 * other.m11;
		float tmp02 = m00 * other.m02 + m01 * other.m12 + m02;
		float tmp10 = m10 * other.m00 + m11 * other.m10;
		float tmp11 = m10 * other.m01 + m11 * other.m11;
		float tmp12 = m10 * other.m02 + m11 * other.m12 + m12;

		m00 = tmp00;
		m01 = tmp01;
		m02 = tmp02;
		m10 = tmp10;
		m11 = tmp11;
		m12 = tmp12;
		return this;
	}

	
	public Affine2 preMul (Affine2 other) {
		float tmp00 = other.m00 * m00 + other.m01 * m10;
		float tmp01 = other.m00 * m01 + other.m01 * m11;
		float tmp02 = other.m00 * m02 + other.m01 * m12 + other.m02;
		float tmp10 = other.m10 * m00 + other.m11 * m10;
		float tmp11 = other.m10 * m01 + other.m11 * m11;
		float tmp12 = other.m10 * m02 + other.m11 * m12 + other.m12;

		m00 = tmp00;
		m01 = tmp01;
		m02 = tmp02;
		m10 = tmp10;
		m11 = tmp11;
		m12 = tmp12;
		return this;
	}

	
	public Affine2 translate (float x, float y) {
		m02 += m00 * x + m01 * y;
		m12 += m10 * x + m11 * y;
		return this;
	}

	
	public Affine2 translate (Vector2 trn) {
		return translate(trn.x, trn.y);
	}

	
	public Affine2 preTranslate (float x, float y) {
		m02 += x;
		m12 += y;
		return this;
	}

	
	public Affine2 preTranslate (Vector2 trn) {
		return preTranslate(trn.x, trn.y);
	}

	
	public Affine2 scale (float scaleX, float scaleY) {
		m00 *= scaleX;
		m01 *= scaleY;
		m10 *= scaleX;
		m11 *= scaleY;
		return this;
	}

	
	public Affine2 scale (Vector2 scale) {
		return scale(scale.x, scale.y);
	}

	
	public Affine2 preScale (float scaleX, float scaleY) {
		m00 *= scaleX;
		m01 *= scaleX;
		m02 *= scaleX;
		m10 *= scaleY;
		m11 *= scaleY;
		m12 *= scaleY;
		return this;
	}

	
	public Affine2 preScale (Vector2 scale) {
		return preScale(scale.x, scale.y);
	}

	
	public Affine2 rotate (float degrees) {
		if (degrees == 0) return this;

		float cos = MathUtils.cosDeg(degrees);
		float sin = MathUtils.sinDeg(degrees);

		float tmp00 = m00 * cos + m01 * sin;
		float tmp01 = m00 * -sin + m01 * cos;
		float tmp10 = m10 * cos + m11 * sin;
		float tmp11 = m10 * -sin + m11 * cos;

		m00 = tmp00;
		m01 = tmp01;
		m10 = tmp10;
		m11 = tmp11;
		return this;
	}

	
	public Affine2 rotateRad (float radians) {
		if (radians == 0) return this;

		float cos = MathUtils.cos(radians);
		float sin = MathUtils.sin(radians);

		float tmp00 = m00 * cos + m01 * sin;
		float tmp01 = m00 * -sin + m01 * cos;
		float tmp10 = m10 * cos + m11 * sin;
		float tmp11 = m10 * -sin + m11 * cos;

		m00 = tmp00;
		m01 = tmp01;
		m10 = tmp10;
		m11 = tmp11;
		return this;
	}

	
	public Affine2 preRotate (float degrees) {
		if (degrees == 0) return this;

		float cos = MathUtils.cosDeg(degrees);
		float sin = MathUtils.sinDeg(degrees);

		float tmp00 = cos * m00 - sin * m10;
		float tmp01 = cos * m01 - sin * m11;
		float tmp02 = cos * m02 - sin * m12;
		float tmp10 = sin * m00 + cos * m10;
		float tmp11 = sin * m01 + cos * m11;
		float tmp12 = sin * m02 + cos * m12;

		m00 = tmp00;
		m01 = tmp01;
		m02 = tmp02;
		m10 = tmp10;
		m11 = tmp11;
		m12 = tmp12;
		return this;
	}

	
	public Affine2 preRotateRad (float radians) {
		if (radians == 0) return this;

		float cos = MathUtils.cos(radians);
		float sin = MathUtils.sin(radians);

		float tmp00 = cos * m00 - sin * m10;
		float tmp01 = cos * m01 - sin * m11;
		float tmp02 = cos * m02 - sin * m12;
		float tmp10 = sin * m00 + cos * m10;
		float tmp11 = sin * m01 + cos * m11;
		float tmp12 = sin * m02 + cos * m12;

		m00 = tmp00;
		m01 = tmp01;
		m02 = tmp02;
		m10 = tmp10;
		m11 = tmp11;
		m12 = tmp12;
		return this;
	}

	
	public Affine2 shear (float shearX, float shearY) {
		float tmp0 = m00 + shearY * m01;
		float tmp1 = m01 + shearX * m00;
		m00 = tmp0;
		m01 = tmp1;

		tmp0 = m10 + shearY * m11;
		tmp1 = m11 + shearX * m10;
		m10 = tmp0;
		m11 = tmp1;
		return this;
	}

	
	public Affine2 shear (Vector2 shear) {
		return shear(shear.x, shear.y);
	}

	
	public Affine2 preShear (float shearX, float shearY) {
		float tmp00 = m00 + shearX * m10;
		float tmp01 = m01 + shearX * m11;
		float tmp02 = m02 + shearX * m12;
		float tmp10 = m10 + shearY * m00;
		float tmp11 = m11 + shearY * m01;
		float tmp12 = m12 + shearY * m02;

		m00 = tmp00;
		m01 = tmp01;
		m02 = tmp02;
		m10 = tmp10;
		m11 = tmp11;
		m12 = tmp12;
		return this;
	}

	
	public Affine2 preShear (Vector2 shear) {
		return preShear(shear.x, shear.y);
	}

	
	public float det () {
		return m00 * m11 - m01 * m10;
	}

	
	public Vector2 getTranslation (Vector2 position) {
		position.x = m02;
		position.y = m12;
		return position;
	}

	
	public boolean isTranslation () {
		return (m00 == 1 && m11 == 1 && m01 == 0 && m10 == 0);
	}

	
	public boolean isIdt () {
		return (m00 == 1 && m02 == 0 && m12 == 0 && m11 == 1 && m01 == 0 && m10 == 0);
	}

	
	public void applyTo (Vector2 point) {
		float x = point.x;
		float y = point.y;
		point.x = m00 * x + m01 * y + m02;
		point.y = m10 * x + m11 * y + m12;
	}

	@Override
	public String toString () {
		return "[" + m00 + "|" + m01 + "|" + m02 + "]\n[" + m10 + "|" + m11 + "|" + m12 + "]\n[0.0|0.0|0.1]";
	}
}
