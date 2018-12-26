

package com.badlogic.gdx.tests.bullet;

import com.badlogic.gdx.graphics.Camera;
import com.badlogic.gdx.math.Frustum;
import com.badlogic.gdx.math.Plane;
import com.badlogic.gdx.math.Vector3;
import com.badlogic.gdx.physics.bullet.collision.*;
import com.badlogic.gdx.utils.BufferUtils;
import com.badlogic.gdx.utils.Disposable;

import java.nio.FloatBuffer;


public abstract class OcclusionCuller implements Disposable {

	protected class Collider extends ICollide {

		
		@Override
		public boolean AllLeaves (btDbvtNode node) {
			if (node.isleaf()) {
				onObjectVisible(node.getDataAsProxyClientObject());
			} else {
				long nodePointer = node.getCPointer();
				btDbvtNode child;
				if ((child = btDbvtNode.internalTemp(nodePointer, false).getChild(0)).getCPointer() != 0) AllLeaves(child);
				if ((child = btDbvtNode.internalTemp(nodePointer, false).getChild(1)).getCPointer() != 0) AllLeaves(child);
			}
			return true;
		}

		
		@Override
		public boolean Descent (btDbvtNode node) {
			return oclBuffer.queryAABB(tmpV1.set(node.getVolume().Center()), tmpV2.set(node.getVolume().Extents()));
		}

		
		@Override
		public void Process (btDbvtNode leaf, float depth) {
			btCollisionObject object = leaf.getDataAsProxyClientObject();
			onObjectVisible(object);
			btCollisionShape shape = object.getCollisionShape();
			if (shape instanceof btBoxShape && isOccluder(object)) {
				oclBuffer.drawBB(object.getWorldTransform(), ((btBoxShape)shape).getHalfExtentsWithMargin());
			}

		}
	}

	private static final int NUM_PLANES = 5;

	private final FloatBuffer frustumNormals = BufferUtils.newFloatBuffer(NUM_PLANES * 4);
	private final FloatBuffer frustumOffsets = BufferUtils.newFloatBuffer(NUM_PLANES);
	final Vector3 tmpV1 = new Vector3();
	final Vector3 tmpV2 = new Vector3();
	OcclusionBuffer oclBuffer;

	private final Collider collider = new Collider();

	@Override
	public void dispose () {
		collider.dispose();
		oclBuffer = null;
	}

	
	public abstract boolean isOccluder (btCollisionObject object);

	
	public abstract void onObjectVisible (btCollisionObject object);

	
	public void performKDOPCulling (btDbvtBroadphase broadphase, Camera camera) {
		setFrustumPlanes(camera.frustum);
		btDbvt.collideKDOP(broadphase.getSet1().getRoot(), frustumNormals, frustumOffsets, NUM_PLANES, collider);
		btDbvt.collideKDOP(broadphase.getSet0().getRoot(), frustumNormals, frustumOffsets, NUM_PLANES, collider);
	}

	
	public void performOcclusionCulling (btDbvtBroadphase broadphase, OcclusionBuffer oclBuffer, Camera camera) {
		this.oclBuffer = oclBuffer;
		oclBuffer.setProjectionMatrix(camera.combined);
		setFrustumPlanes(camera.frustum);
		btDbvt.collideOCL(broadphase.getSet1().getRoot(), frustumNormals, frustumOffsets, camera.direction, NUM_PLANES, collider);
		btDbvt.collideOCL(broadphase.getSet0().getRoot(), frustumNormals, frustumOffsets, camera.direction, NUM_PLANES, collider);
	}

	
	private void setFrustumPlanes (Frustum frustum) {
				frustumNormals.clear();
		frustumOffsets.clear();
		for (int i = 1; i < 6; i++) {
			Plane plane = frustum.planes[i];
									frustumNormals.put(plane.normal.x);
			frustumNormals.put(plane.normal.y);
			frustumNormals.put(plane.normal.z);
			frustumNormals.put(0);
			frustumOffsets.put(plane.d);
		}
	}
}
