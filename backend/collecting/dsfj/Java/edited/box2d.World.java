

package com.badlogic.gdx.physics.box2d;

import java.util.Iterator;

import org.jbox2d.collision.AABB;
import org.jbox2d.common.Settings;
import org.jbox2d.common.Vec2;
import org.jbox2d.dynamics.joints.JointEdge;

import com.badlogic.gdx.math.Vector2;
import com.badlogic.gdx.physics.box2d.BodyDef.BodyType;
import com.badlogic.gdx.physics.box2d.JointDef.JointType;
import com.badlogic.gdx.physics.box2d.joints.DistanceJoint;
import com.badlogic.gdx.physics.box2d.joints.FrictionJoint;
import com.badlogic.gdx.physics.box2d.joints.GearJoint;
import com.badlogic.gdx.physics.box2d.joints.GearJointDef;
import com.badlogic.gdx.physics.box2d.joints.MotorJoint;
import com.badlogic.gdx.physics.box2d.joints.MouseJoint;
import com.badlogic.gdx.physics.box2d.joints.PrismaticJoint;
import com.badlogic.gdx.physics.box2d.joints.PulleyJoint;
import com.badlogic.gdx.physics.box2d.joints.RevoluteJoint;
import com.badlogic.gdx.physics.box2d.joints.RopeJoint;
import com.badlogic.gdx.physics.box2d.joints.WeldJoint;
import com.badlogic.gdx.physics.box2d.joints.WheelJoint;
import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.ObjectMap;


public final class World implements Disposable {
	org.jbox2d.dynamics.World world;
	Vec2 tmp = new Vec2();
	Vector2 tmp2 = new Vector2();
	ObjectMap<org.jbox2d.dynamics.Body, Body> bodies = new ObjectMap<org.jbox2d.dynamics.Body, Body>();
	ObjectMap<org.jbox2d.dynamics.Fixture, Fixture> fixtures = new ObjectMap<org.jbox2d.dynamics.Fixture, Fixture>();
	ObjectMap<org.jbox2d.dynamics.joints.Joint, Joint> joints = new ObjectMap<org.jbox2d.dynamics.joints.Joint, Joint>();

	
	public World (Vector2 gravity, boolean doSleep) {
		world = new org.jbox2d.dynamics.World(tmp.set(gravity.x, gravity.y));
		world.setAllowSleep(doSleep);
	}

	
	public void setDestructionListener (DestructionListener listener) {
	}

	
	public void setContactFilter (final ContactFilter filter) {
		if (filter != null) {
			world.setContactFilter(new org.jbox2d.callbacks.ContactFilter() {
				@Override
				public boolean shouldCollide (org.jbox2d.dynamics.Fixture fixtureA, org.jbox2d.dynamics.Fixture fixtureB) {
					return filter.shouldCollide(fixtures.get(fixtureA), fixtures.get(fixtureB));
				}
			});
		} else {
			world.setContactFilter(new org.jbox2d.callbacks.ContactFilter());
		}
	}

	
	Contact tmpContact = new Contact(this);
	Manifold tmpManifold = new Manifold();
	ContactImpulse tmpImpulse = new ContactImpulse();

	public void setContactListener (final ContactListener listener) {
		if (listener != null) {
			world.setContactListener(new org.jbox2d.callbacks.ContactListener() {
				@Override
				public void beginContact (org.jbox2d.dynamics.contacts.Contact contact) {
					tmpContact.contact = contact;
					listener.beginContact(tmpContact);
				}

				@Override
				public void endContact (org.jbox2d.dynamics.contacts.Contact contact) {
					tmpContact.contact = contact;
					listener.endContact(tmpContact);
				}

				@Override
				public void preSolve (org.jbox2d.dynamics.contacts.Contact contact, org.jbox2d.collision.Manifold oldManifold) {
					tmpContact.contact = contact;
					tmpManifold.manifold = oldManifold;
					listener.preSolve(tmpContact, tmpManifold);
				}

				@Override
				public void postSolve (org.jbox2d.dynamics.contacts.Contact contact, org.jbox2d.callbacks.ContactImpulse impulse) {
					tmpContact.contact = contact;
					tmpImpulse.impulse = impulse;
					listener.postSolve(tmpContact, tmpImpulse);
				}
			});
		} else {
			world.setContactListener(null);
		}
	}

	
	public Body createBody (BodyDef def) {
		org.jbox2d.dynamics.BodyDef bd = new org.jbox2d.dynamics.BodyDef();
		bd.active = def.active;
		bd.allowSleep = def.allowSleep;
		bd.angle = def.angle;
		bd.angularDamping = def.angularDamping;
		bd.angularVelocity = def.angularVelocity;
		bd.awake = def.awake;
		bd.bullet = def.bullet;
		bd.fixedRotation = def.fixedRotation;
		bd.gravityScale = def.gravityScale;
		bd.linearDamping = def.linearDamping;
		bd.linearVelocity.set(def.linearVelocity.x, def.linearVelocity.y);
		bd.position.set(def.position.x, def.position.y);
		if (def.type == BodyType.DynamicBody) bd.type = org.jbox2d.dynamics.BodyType.DYNAMIC;
		if (def.type == BodyType.StaticBody) bd.type = org.jbox2d.dynamics.BodyType.STATIC;
		if (def.type == BodyType.KinematicBody) bd.type = org.jbox2d.dynamics.BodyType.KINEMATIC;

		org.jbox2d.dynamics.Body b = world.createBody(bd);
		Body body = new Body(this, b);
		bodies.put(b, body);
		return body;
	}

	
	public void destroyBody (Body body) {
		JointEdge jointEdge = body.body.getJointList();
		while (jointEdge != null) {
			JointEdge next = jointEdge.next;			
			world.destroyJoint(jointEdge.joint);
			joints.remove(jointEdge.joint);
			jointEdge = next;
		}
		world.destroyBody(body.body);
		bodies.remove(body.body);
		for (Fixture fixture : body.fixtures) {
			fixtures.remove(fixture.fixture);
		}
	}

	
	public Joint createJoint (JointDef def) {
		org.jbox2d.dynamics.joints.JointDef jd = def.toJBox2d();
		org.jbox2d.dynamics.joints.Joint j = world.createJoint(jd);
		Joint joint = null;
		if (def.type == JointType.DistanceJoint) joint = new DistanceJoint(this, (org.jbox2d.dynamics.joints.DistanceJoint)j);
		if (def.type == JointType.FrictionJoint) joint = new FrictionJoint(this, (org.jbox2d.dynamics.joints.FrictionJoint)j);
		if (def.type == JointType.GearJoint) joint = new GearJoint(this, (org.jbox2d.dynamics.joints.GearJoint)j, ((GearJointDef) def).joint1, ((GearJointDef) def).joint2);
		if (def.type == JointType.MotorJoint) joint = new MotorJoint(this, (org.jbox2d.dynamics.joints.MotorJoint)j);
		if (def.type == JointType.MouseJoint) joint = new MouseJoint(this, (org.jbox2d.dynamics.joints.MouseJoint)j);
		if (def.type == JointType.PrismaticJoint) joint = new PrismaticJoint(this, (org.jbox2d.dynamics.joints.PrismaticJoint)j);
		if (def.type == JointType.PulleyJoint) joint = new PulleyJoint(this, (org.jbox2d.dynamics.joints.PulleyJoint)j);
		if (def.type == JointType.RevoluteJoint) joint = new RevoluteJoint(this, (org.jbox2d.dynamics.joints.RevoluteJoint)j);
		if (def.type == JointType.RopeJoint) joint = new RopeJoint(this, (org.jbox2d.dynamics.joints.RopeJoint)j);
		if (def.type == JointType.WeldJoint) joint = new WeldJoint(this, (org.jbox2d.dynamics.joints.WeldJoint)j);
		if (def.type == JointType.WheelJoint) joint = new WheelJoint(this, (org.jbox2d.dynamics.joints.WheelJoint)j);
		if (joint == null) throw new GdxRuntimeException("Joint type '" + def.type + "' not yet supported by GWT backend");
		joints.put(j, joint);
		return joint;
	}

	
	public void destroyJoint (Joint joint) {
		joint.setUserData(null);
		world.destroyJoint(joint.joint);
		joints.remove(joint.joint);
	}

	
	public void step (float timeStep, int velocityIterations, int positionIterations) {
		world.step(timeStep, velocityIterations, positionIterations);
	}

	
	public void clearForces () {
		world.clearForces();
	}

	
	public void setWarmStarting (boolean flag) {
		world.setWarmStarting(flag);
	}

	
	public void setContinuousPhysics (boolean flag) {
		world.setContinuousPhysics(flag);
	}

	
	public int getProxyCount () {
		return world.getProxyCount();
	}

	
	public int getBodyCount () {
		return world.getBodyCount();
	}

	
	public int getJointCount () {
		return world.getJointCount();
	}

	
	public int getContactCount () {
		return world.getContactCount();
	}

	
	public void setGravity (Vector2 gravity) {
		world.setGravity(tmp.set(gravity.x, gravity.y));
	}

	public Vector2 getGravity () {
		Vec2 gravity = world.getGravity();
		return tmp2.set(gravity.x, gravity.y);
	}

	
	public boolean isLocked () {
		return world.isLocked();
	}

	
	public void setAutoClearForces (boolean flag) {
		world.setAutoClearForces(flag);
	}

	
	public boolean getAutoClearForces () {
		return world.getAutoClearForces();
	}

	
	AABB aabb = new AABB();

	public void QueryAABB (final QueryCallback callback, float lowerX, float lowerY, float upperX, float upperY) {
				aabb.lowerBound.set(lowerX, lowerY);
		aabb.upperBound.set(upperX, upperY);
		world.queryAABB(new org.jbox2d.callbacks.QueryCallback() {
			@Override
			public boolean reportFixture (org.jbox2d.dynamics.Fixture f) {
				Fixture fixture = fixtures.get(f);
				return callback.reportFixture(fixture);
			}
		}, aabb);
	}

	
	Array<Contact> contacts = new Array<Contact>();

	public Array<Contact> getContactList () {
				org.jbox2d.dynamics.contacts.Contact contactList = world.getContactList();
		contacts.clear();
		while (contactList != null) {
			Contact contact = new Contact(this, contactList);
			contacts.add(contact);
			contactList = contactList.m_next;
		}
		return contacts;
	}

	
	public void getBodies (Array<Body> bodies) {
		bodies.clear();
		bodies.ensureCapacity(this.bodies.size);
		for (Iterator<Body> iter = this.bodies.values(); iter.hasNext();) {
			bodies.add(iter.next());
		}
	}

	
	public void getJoints (Array<Joint> joints) {
		joints.clear();
		joints.ensureCapacity(this.joints.size);
		for (Iterator<Joint> iter = this.joints.values(); iter.hasNext();) {
			joints.add(iter.next());
		}
	}

	public void dispose () {
	}

	
	public static void setVelocityThreshold (float threshold) {
		Settings.velocityThreshold = threshold;
	}

	
	public static float getVelocityThreshold () {
		return Settings.velocityThreshold;
	}

	
	Vec2 point1 = new Vec2();
	Vec2 point2 = new Vec2();
	Vector2 point = new Vector2();
	Vector2 normal = new Vector2();

	public void rayCast (final RayCastCallback callback, Vector2 point1, Vector2 point2) {
		rayCast(callback, point1.x, point1.y, point2.x, point2.y);
	}

	public void rayCast (final RayCastCallback callback, float point1X, float point1Y, float point2X, float point2Y) {
				world.raycast(new org.jbox2d.callbacks.RayCastCallback() {
			@Override
			public float reportFixture (org.jbox2d.dynamics.Fixture f, Vec2 p, Vec2 n, float fraction) {
				return callback.reportRayFixture(fixtures.get(f), point.set(p.x, p.y), normal.set(n.x, n.y), fraction);
			}
		}, this.point1.set(point1X, point1Y), this.point2.set(point2X, point2Y));
	}
}
