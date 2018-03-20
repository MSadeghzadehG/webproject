

package com.badlogic.gdx.physics.box2d.graphics;

import java.io.BufferedReader;
import java.io.IOException;

import com.badlogic.gdx.graphics.g2d.ParticleEmitter;
import com.badlogic.gdx.graphics.g2d.Sprite;
import com.badlogic.gdx.graphics.g2d.ParticleEmitter.Particle;
import com.badlogic.gdx.math.MathUtils;
import com.badlogic.gdx.math.Vector2;
import com.badlogic.gdx.physics.box2d.Fixture;
import com.badlogic.gdx.physics.box2d.RayCastCallback;
import com.badlogic.gdx.physics.box2d.World;


public class ParticleEmitterBox2D extends ParticleEmitter {
	final World world;
	final Vector2 startPoint = new Vector2();
	final Vector2 endPoint = new Vector2();
	
	boolean particleCollided;
	float normalAngle;
	
	private final static float EPSILON = 0.001f;

	
	final RayCastCallback rayCallBack = new RayCastCallback() {
		public float reportRayFixture (Fixture fixture, Vector2 point, Vector2 normal, float fraction) {
			ParticleEmitterBox2D.this.particleCollided = true;
			ParticleEmitterBox2D.this.normalAngle = MathUtils.atan2(normal.y, normal.x) * MathUtils.radiansToDegrees;
			return fraction;
		}
	};

	
	public ParticleEmitterBox2D (World world) {
		super();
		this.world = world;
	}

	
	public ParticleEmitterBox2D (World world, BufferedReader reader) throws IOException {
		super(reader);
		this.world = world;
	}

	
	public ParticleEmitterBox2D (World world, ParticleEmitter emitter) {
		super(emitter);
		this.world = world;
	}

	@Override
	protected Particle newParticle (Sprite sprite) {
		return new ParticleBox2D(sprite);
	}

	
	private class ParticleBox2D extends Particle {
		public ParticleBox2D (Sprite sprite) {
			super(sprite);
		}

		
		@Override
		public void translate (float velocityX, float velocityY) {
			
			if ((velocityX * velocityX + velocityY * velocityY) < EPSILON) return;

			
			final float x = getX() + getWidth() / 2f;
			final float y = getY() + getHeight() / 2f;

			
			particleCollided = false;
			startPoint.set(x, y);
			endPoint.set(x + velocityX, y + velocityY);
			if (world != null) world.rayCast(rayCallBack, startPoint, endPoint);

			
			if (particleCollided) {
								angle = 2f * normalAngle - angle - 180f;
				angleCos = MathUtils.cosDeg(angle);
				angleSin = MathUtils.sinDeg(angle);
				velocityX *= angleCos;
				velocityY *= angleSin;
			}

			super.translate(velocityX, velocityY);
		}
	}
}
