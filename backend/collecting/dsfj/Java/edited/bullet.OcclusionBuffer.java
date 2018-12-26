

package com.badlogic.gdx.tests.bullet;

import com.badlogic.gdx.graphics.Color;
import com.badlogic.gdx.graphics.Pixmap;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.TextureRegion;
import com.badlogic.gdx.math.*;
import com.badlogic.gdx.utils.BufferUtils;
import com.badlogic.gdx.utils.Disposable;

import java.nio.FloatBuffer;


public class OcclusionBuffer implements Disposable {

	static class GridPoint3 extends com.badlogic.gdx.math.GridPoint3 {
		public GridPoint3 add (GridPoint3 other) {
			return (GridPoint3)set(x + other.x, y + other.y, z + other.z);
		}
	}

	
	private enum Policy {
		DRAW, QUERY;

		
		boolean evaluate (Quaternion[] vertices) {
			switch (this) {
				case DRAW:
					return false;
				case QUERY:
																				for (Quaternion vertex : vertices) {
						if (vertex.z + vertex.w <= 0) return true;
					}
					return false;
			}
			return false;
		}

		
		boolean process (FloatBuffer depthBuffer, int bufferIndex, float newDepth) {
			float oldDepth = depthBuffer.get(bufferIndex);
			switch (this) {
				case DRAW:
					if (newDepth > oldDepth) depthBuffer.put(bufferIndex, newDepth);
					return false;
				case QUERY:
					return (newDepth >= oldDepth);
			}
			return false;
		}
	}

	static class Quaternion extends com.badlogic.gdx.math.Quaternion {
		
		public Quaternion mul (final Matrix4 matrix) {
			final float[] val = matrix.val;
			return this.set(x * val[Matrix4.M00] + y * val[Matrix4.M01] + z * val[Matrix4.M02] + w * val[Matrix4.M03],
					x * val[Matrix4.M10] + y * val[Matrix4.M11] + z * val[Matrix4.M12] + w * val[Matrix4.M13],
					x * val[Matrix4.M20] + y * val[Matrix4.M21] + z * val[Matrix4.M22] + w * val[Matrix4.M23],
					x * val[Matrix4.M30] + y * val[Matrix4.M31] + z * val[Matrix4.M32] + w * val[Matrix4.M33]);
		}

		
		public Quaternion mulAdd (final Quaternion quaternion, float scalar) {
			this.x += quaternion.x * scalar;
			this.y += quaternion.y * scalar;
			this.z += quaternion.z * scalar;
			this.w += quaternion.w * scalar;
			return this;
		}

		@Override
		public Quaternion set (float x, float y, float z, float w) {
			return (Quaternion)super.set(x, y, z, w);
		}

		public Quaternion set (Quaternion quaternion) {
			return (Quaternion)super.set(quaternion);
		}

		
		public Quaternion sub (float qx, float qy, float qz, float qw) {
			this.x -= qx;
			this.y -= qy;
			this.z -= qz;
			this.w -= qw;
			return this;
		}

		
		public Quaternion sub (Quaternion quaternion) {
			this.x -= quaternion.x;
			this.y -= quaternion.y;
			this.z -= quaternion.z;
			this.w -= quaternion.w;
			return this;
		}
	}

	
	private static final int[] WINDING = {1, 0, 3, 2, 4, 5, 6, 7, 4, 7, 3, 0, 6, 5, 1, 2, 7, 6, 2, 3, 5, 4, 0, 1};
	
	private final FloatBuffer buffer;
	
	private final Vector2 bufferHalfExt;
	
	private final Vector2 bufferOffset;
	
	public final int bufferWidth;
	
	public final int bufferHeight;

		private final Vector3[] box = new Vector3[8];
	private final Quaternion[] tmpVertices = new Quaternion[8];
	private final Quaternion[] clippedQuad = new Quaternion[8];
	private final Quaternion[] quad = new Quaternion[4];
	private final Quaternion tmpQ1 = new Quaternion();
	private final Quaternion tmpQ2 = new Quaternion();
	private final Vector3 tmpV1 = new Vector3();
	private final Vector3 tmpV2 = new Vector3();
	private final GridPoint3 triX = new GridPoint3();
	private final GridPoint3 triY = new GridPoint3();
	private final GridPoint3 triDX = new GridPoint3();
	private final GridPoint3 triDY = new GridPoint3();
	private final GridPoint3 cursor = new GridPoint3();

		private Pixmap debugPixmap;
	private Texture debugTexture;
	private TextureRegion debugTextureRegion;
	private Matrix4 projectionMatrix = new Matrix4();

	
	public OcclusionBuffer (int width, int height) {
		bufferWidth = width;
		bufferHeight = height;
		bufferHalfExt = new Vector2(width * 0.5f, height * 0.5f);
		bufferOffset = new Vector2(bufferHalfExt.x + 0.5f, bufferHalfExt.y + 0.5f);
		buffer = BufferUtils.newFloatBuffer(width * height);
		for (int i = 0; i < 8; i++) {
			box[i] = new Vector3();
			tmpVertices[i] = new Quaternion();
			clippedQuad[i] = new Quaternion();
		}
		for (int i = 0; i < 4; i++)
			quad[i] = new Quaternion();
	}

	
	public void clear () {
		buffer.clear();
		while (buffer.position() < buffer.capacity())
			buffer.put(-1);
	}

	
	private int clipQuad (Quaternion[] verticesIn, Quaternion[] verticesOut) {
		int numVerts = verticesIn.length;
		int numVertsBehind = 0;
		float[] s = new float[4];
		for (int i = 0; i < numVerts; i++) {
			s[i] = verticesIn[i].z + verticesIn[i].w;
			if (s[i] < 0) numVertsBehind++;
		}
		if (numVertsBehind == numVerts) {
						return 0;

		} else if (numVertsBehind > 0) {
						int newNumVerts = 0;
			for (int i = numVerts - 1, j = 0; j < numVerts; i = j++) {
				Quaternion a = tmpQ1.set(verticesIn[i]);
				Quaternion b = tmpQ2.set(verticesIn[j]);
				float t = s[i] / (a.w + a.z - b.w - b.z);
				if ((t > 0) && (t < 1)) verticesOut[newNumVerts++].set(a).mulAdd(b.sub(a), t);
				if (s[j] > 0) verticesOut[newNumVerts++].set(verticesIn[j]);
			}
			return newNumVerts;

		} else {
						for (int i = 0; i < numVerts; i++)
				verticesOut[i].set(verticesIn[i]);
			return numVerts;
		}
	}

	@Override
	public void dispose () {
		if (debugPixmap != null) {
			debugPixmap.dispose();
			debugTexture.dispose();
			debugPixmap = null;
			debugTexture = null;
		}
	}

	
	public void drawAABB (Vector3 center, Vector3 halfExt) {
		setAABBVertices(center, halfExt, box);
		drawBox(box, Policy.DRAW);
	}

	
	public void drawBB (Matrix4 worldTransform, Vector3 halfExt) {
		Vector3 center = tmpV1.setZero();
		setAABBVertices(center, halfExt, box);
		worldTransform.getTranslation(center);
		for (Vector3 vertex : box) {
			vertex.rot(worldTransform);
			vertex.add(center);
		}
		drawBox(box, Policy.DRAW);
	}

	
	private boolean drawBox (Vector3[] vertices, Policy policy) {
		for (int i = 0; i < 8; i++) {
						Vector3 v = vertices[i];
			tmpVertices[i].set(v.x, v.y, v.z, 1).mul(projectionMatrix);
		}
		if (policy.evaluate(tmpVertices)) return true;

				for (int i = 0; i < WINDING.length;) {
			quad[0].set(tmpVertices[WINDING[i++]]);
			quad[1].set(tmpVertices[WINDING[i++]]);
			quad[2].set(tmpVertices[WINDING[i++]]);
			quad[3].set(tmpVertices[WINDING[i++]]);
						int numVertices = clipQuad(quad, clippedQuad);
						for (int j = 0; j < numVertices; j++) {
				Quaternion q = clippedQuad[j];
				q.z = 1 / q.w;
				vertices[j].set(q.x * q.z, q.y * q.z, q.z);
			}
						for (int j = 2; j < numVertices; j++) {
												if (drawTriangle(vertices[0], vertices[j - 1], vertices[j], policy)) return true;
			}
		}
		return false;
	}

	
	public TextureRegion drawDebugTexture () {
		if (debugPixmap == null) {
			debugPixmap = new Pixmap(bufferWidth, bufferHeight, Pixmap.Format.RGBA8888);
			debugTexture = new Texture(debugPixmap);
			debugTextureRegion = new TextureRegion(debugTexture);
			debugTextureRegion.flip(false, true);
		}
		debugPixmap.setColor(Color.BLACK);
		debugPixmap.fill();
				float minDepth = Float.POSITIVE_INFINITY;
		float maxDepth = Float.NEGATIVE_INFINITY;
		buffer.clear();
		while (buffer.position() < buffer.capacity()) {
			float depth = MathUtils.clamp(buffer.get(), 0, Float.POSITIVE_INFINITY);
			minDepth = Math.min(depth, minDepth);
			maxDepth = Math.max(depth, maxDepth);
		}
		float extent = 1 / (maxDepth - minDepth);
		buffer.clear();
				for (int x = 0; x < bufferWidth; x++) {
			for (int y = 0; y < bufferHeight; y++) {
				float depth = MathUtils.clamp(buffer.get(x + y * bufferWidth), 0, Float.POSITIVE_INFINITY);
				float c = depth * extent;
				debugPixmap.drawPixel(x, y, Color.rgba8888(c, c, c, 1));
			}
		}
		debugTexture.draw(debugPixmap, 0, 0);
		return debugTextureRegion;
	}

	
	private boolean drawTriangle (Vector3 a, Vector3 b, Vector3 c, Policy policy) {
				if (((tmpV1.set(b).sub(a)).crs(tmpV2.set(c).sub(a))).z <= 0) return false;
								triX.set((int)(a.x * bufferHalfExt.x + bufferOffset.x), (int)(b.x * bufferHalfExt.x + bufferOffset.x),
				(int)(c.x * bufferHalfExt.x + bufferOffset.x));
		triY.set((int)(a.y * bufferHalfExt.y + bufferOffset.y), (int)(b.y * bufferHalfExt.y + bufferOffset.y),
				(int)(c.y * bufferHalfExt.y + bufferOffset.y));
				int xMin = Math.max(0, Math.min(triX.x, Math.min(triX.y, triX.z)));
		int xMax = Math.min(bufferWidth, 1 + Math.max(triX.x, Math.max(triX.y, triX.z)));
		int yMin = Math.max(0, Math.min(triY.x, Math.min(triY.y, triY.z)));
		int yMax = Math.min(bufferWidth, 1 + Math.max(triY.x, Math.max(triY.y, triY.z)));
		int width = xMax - xMin;
		int height = yMax - yMin;
		if (width * height <= 0) return false;
				triDX.set(triY.x - triY.y, triY.y - triY.z, triY.z - triY.x);
		triDY.set(triX.y - triX.x - triDX.x * width, triX.z - triX.y - triDX.y * width, triX.x - triX.z - triDX.z * width);
		cursor.set(yMin * (triX.y - triX.x) + xMin * (triY.x - triY.y) + triX.x * triY.y - triX.y * triY.x,
				yMin * (triX.z - triX.y) + xMin * (triY.y - triY.z) + triX.y * triY.z - triX.z * triY.y,
				yMin * (triX.x - triX.z) + xMin * (triY.z - triY.x) + triX.z * triY.x - triX.x * triY.z);
				float ia = 1f
				/ (float)(triX.x * triY.y - triX.y * triY.x + triX.z * triY.x - triX.x * triY.z + triX.y * triY.z - triX.z * triY.y);
		float dzx = ia * (triY.x * (c.z - b.z) + triY.y * (a.z - c.z) + triY.z * (b.z - a.z));
		float dzy = ia * (triX.x * (b.z - c.z) + triX.y * (c.z - a.z) + triX.z * (a.z - b.z)) - (dzx * width);
		float drawDepth = ia * (a.z * cursor.y + b.z * cursor.z + c.z * cursor.x);
		int bufferRow = (yMin * bufferHeight);
				for (int iy = yMin; iy < yMax; iy++) {
			for (int ix = xMin; ix < xMax; ix++) {
				int bufferIndex = bufferRow + ix;
				if (cursor.x >= 0 && cursor.y >= 0 && cursor.z >= 0 && policy.process(buffer, bufferIndex, drawDepth)) return true;
				cursor.add(triDX);
				drawDepth += dzx;
			}
			cursor.add(triDY);
			drawDepth += dzy;
			bufferRow += bufferWidth;
		}
		return false;
	}

	
	public boolean queryAABB (Vector3 center, Vector3 halfExt) {
		setAABBVertices(center, halfExt, box);
		return drawBox(box, Policy.QUERY);
	}

	
	private static void setAABBVertices (Vector3 center, Vector3 halfExt, Vector3[] vertices) {
		vertices[0].set(center.x - halfExt.x, center.y - halfExt.y, center.z - halfExt.z);
		vertices[1].set(center.x + halfExt.x, center.y - halfExt.y, center.z - halfExt.z);
		vertices[2].set(center.x + halfExt.x, center.y + halfExt.y, center.z - halfExt.z);
		vertices[3].set(center.x - halfExt.x, center.y + halfExt.y, center.z - halfExt.z);
		vertices[4].set(center.x - halfExt.x, center.y - halfExt.y, center.z + halfExt.z);
		vertices[5].set(center.x + halfExt.x, center.y - halfExt.y, center.z + halfExt.z);
		vertices[6].set(center.x + halfExt.x, center.y + halfExt.y, center.z + halfExt.z);
		vertices[7].set(center.x - halfExt.x, center.y + halfExt.y, center.z + halfExt.z);
	}

	
	public void setProjectionMatrix (Matrix4 matrix) {
		projectionMatrix.set(matrix);
	}

}
