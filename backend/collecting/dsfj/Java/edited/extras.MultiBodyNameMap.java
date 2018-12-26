

package com.badlogic.gdx.physics.bullet.extras;

import com.badlogic.gdx.physics.bullet.BulletBase;
import com.badlogic.gdx.physics.bullet.linearmath.*;
import com.badlogic.gdx.physics.bullet.collision.*;
import com.badlogic.gdx.physics.bullet.dynamics.*;
import com.badlogic.gdx.math.Vector3;
import com.badlogic.gdx.math.Quaternion;
import com.badlogic.gdx.math.Matrix3;
import com.badlogic.gdx.math.Matrix4;
import com.badlogic.gdx.physics.bullet.inversedynamics.MultiBodyTree;
import com.badlogic.gdx.physics.bullet.dynamics.btDynamicsWorld;
import com.badlogic.gdx.physics.bullet.dynamics.btContactSolverInfo;
import com.badlogic.gdx.physics.bullet.collision.btCollisionShape;

public class MultiBodyNameMap extends BulletBase {
	private long swigCPtr;
	
	protected MultiBodyNameMap(final String className, long cPtr, boolean cMemoryOwn) {
		super(className, cPtr, cMemoryOwn);
		swigCPtr = cPtr;
	}
	
	 
	public MultiBodyNameMap(long cPtr, boolean cMemoryOwn) {
		this("MultiBodyNameMap", cPtr, cMemoryOwn);
		construct();
	}
	
	@Override
	protected void reset(long cPtr, boolean cMemoryOwn) {
		if (!destroyed)
			destroy();
		super.reset(swigCPtr = cPtr, cMemoryOwn);
	}
	
	public static long getCPtr(MultiBodyNameMap obj) {
		return (obj == null) ? 0 : obj.swigCPtr;
	}

	@Override
	protected void finalize() throws Throwable {
		if (!destroyed)
			destroy();
		super.finalize();
	}

  @Override protected synchronized void delete() {
		if (swigCPtr != 0) {
			if (swigCMemOwn) {
				swigCMemOwn = false;
				ExtrasJNI.delete_MultiBodyNameMap(swigCPtr);
			}
			swigCPtr = 0;
		}
		super.delete();
	}

  public MultiBodyNameMap() {
    this(ExtrasJNI.new_MultiBodyNameMap(), true);
  }

  public int addBody(int index, SWIGTYPE_p_std__string name) {
    return ExtrasJNI.MultiBodyNameMap_addBody(swigCPtr, this, index, SWIGTYPE_p_std__string.getCPtr(name));
  }

  public int addJoint(int index, SWIGTYPE_p_std__string name) {
    return ExtrasJNI.MultiBodyNameMap_addJoint(swigCPtr, this, index, SWIGTYPE_p_std__string.getCPtr(name));
  }

  public int getBodyName(int index, SWIGTYPE_p_std__string name) {
    return ExtrasJNI.MultiBodyNameMap_getBodyName(swigCPtr, this, index, SWIGTYPE_p_std__string.getCPtr(name));
  }

  public int getJointName(int index, SWIGTYPE_p_std__string name) {
    return ExtrasJNI.MultiBodyNameMap_getJointName(swigCPtr, this, index, SWIGTYPE_p_std__string.getCPtr(name));
  }

  public int getBodyIndex(SWIGTYPE_p_std__string name, java.nio.IntBuffer index) {
    assert index.isDirect() : "Buffer must be allocated direct.";
    {
      return ExtrasJNI.MultiBodyNameMap_getBodyIndex(swigCPtr, this, SWIGTYPE_p_std__string.getCPtr(name), index);
    }
  }

  public int getJointIndex(SWIGTYPE_p_std__string name, java.nio.IntBuffer index) {
    assert index.isDirect() : "Buffer must be allocated direct.";
    {
      return ExtrasJNI.MultiBodyNameMap_getJointIndex(swigCPtr, this, SWIGTYPE_p_std__string.getCPtr(name), index);
    }
  }

}
