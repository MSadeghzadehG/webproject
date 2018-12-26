

package com.badlogic.gdx.tests;

import java.util.ArrayList;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.InputProcessor;
import com.badlogic.gdx.graphics.Color;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.OrthographicCamera;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.BitmapFont;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;
import com.badlogic.gdx.graphics.g2d.TextureRegion;
import com.badlogic.gdx.graphics.glutils.ShapeRenderer;
import com.badlogic.gdx.graphics.glutils.ShapeRenderer.ShapeType;
import com.badlogic.gdx.math.MathUtils;
import com.badlogic.gdx.math.Matrix4;
import com.badlogic.gdx.math.Vector2;
import com.badlogic.gdx.math.Vector3;
import com.badlogic.gdx.physics.box2d.Body;
import com.badlogic.gdx.physics.box2d.BodyDef;
import com.badlogic.gdx.physics.box2d.BodyDef.BodyType;
import com.badlogic.gdx.physics.box2d.Box2DDebugRenderer;
import com.badlogic.gdx.physics.box2d.ChainShape;
import com.badlogic.gdx.physics.box2d.Contact;
import com.badlogic.gdx.physics.box2d.ContactImpulse;
import com.badlogic.gdx.physics.box2d.ContactListener;
import com.badlogic.gdx.physics.box2d.Fixture;
import com.badlogic.gdx.physics.box2d.FixtureDef;
import com.badlogic.gdx.physics.box2d.Manifold;
import com.badlogic.gdx.physics.box2d.PolygonShape;
import com.badlogic.gdx.physics.box2d.QueryCallback;
import com.badlogic.gdx.physics.box2d.World;
import com.badlogic.gdx.physics.box2d.WorldManifold;
import com.badlogic.gdx.physics.box2d.joints.MouseJoint;
import com.badlogic.gdx.physics.box2d.joints.MouseJointDef;
import com.badlogic.gdx.tests.utils.GdxTest;
import com.badlogic.gdx.utils.TimeUtils;

public class Box2DTest extends GdxTest implements InputProcessor {
	
	private com.badlogic.gdx.graphics.OrthographicCamera camera;

	
	private ShapeRenderer renderer;

	
	private Box2DDebugRenderer debugRenderer;

	
	private SpriteBatch batch;
	private BitmapFont font;
	private TextureRegion textureRegion;

	
	private World world;

	
	private ArrayList<Body> boxes = new ArrayList<Body>();

	
	Body groundBody;

	
	private MouseJoint mouseJoint = null;

	
	Body hitBody = null;

	@Override
	public void create () {
																camera = new OrthographicCamera(48, 32);
		camera.position.set(0, 16, 0);

				renderer = new ShapeRenderer();

				debugRenderer = new Box2DDebugRenderer();

				batch = new SpriteBatch();
		font = new BitmapFont(Gdx.files.internal("data/arial-15.fnt"), false);
		font.setColor(Color.RED);
		textureRegion = new TextureRegion(new Texture(Gdx.files.internal("data/badlogicsmall.jpg")));

				createPhysicsWorld();

				Gdx.input.setInputProcessor(this);
	}

	private void createPhysicsWorld () {
						world = new World(new Vector2(0, -10), true);

		float[] vertices = {-0.07421887f, -0.16276085f, -0.12109375f, -0.22786504f, -0.157552f, -0.7122401f, 0.04296875f,
			-0.7122401f, 0.110677004f, -0.6419276f, 0.13151026f, -0.49869835f, 0.08984375f, -0.3190109f};

		PolygonShape shape = new PolygonShape();
		shape.set(vertices);

														PolygonShape groundPoly = new PolygonShape();
		groundPoly.setAsBox(50, 1);

						BodyDef groundBodyDef = new BodyDef();
		groundBodyDef.type = BodyType.StaticBody;
		groundBody = world.createBody(groundBodyDef);

										FixtureDef fixtureDef = new FixtureDef();
		fixtureDef.shape = groundPoly;
		fixtureDef.filter.groupIndex = 0;
		groundBody.createFixture(fixtureDef);
		groundPoly.dispose();

						ChainShape chainShape = new ChainShape();
		chainShape.createLoop(new Vector2[] {new Vector2(-10, 10), new Vector2(-10, 5), new Vector2(10, 5), new Vector2(10, 11),});
		BodyDef chainBodyDef = new BodyDef();
		chainBodyDef.type = BodyType.StaticBody;
		Body chainBody = world.createBody(chainBodyDef);
		chainBody.createFixture(chainShape, 0);
		chainShape.dispose();

		createBoxes();

				world.setContactListener(new ContactListener() {
			@Override
			public void beginContact (Contact contact) {
			}

			@Override
			public void endContact (Contact contact) {
			}

			@Override
			public void preSolve (Contact contact, Manifold oldManifold) {
			}

			@Override
			public void postSolve (Contact contact, ContactImpulse impulse) {
			}
		});
	}

	private void createBoxes () {
								PolygonShape boxPoly = new PolygonShape();
		boxPoly.setAsBox(1, 1);

								for (int i = 0; i < 20; i++) {
									BodyDef boxBodyDef = new BodyDef();
			boxBodyDef.type = BodyType.DynamicBody;
			boxBodyDef.position.x = -24 + (float)(Math.random() * 48);
			boxBodyDef.position.y = 10 + (float)(Math.random() * 100);
			Body boxBody = world.createBody(boxBodyDef);

			boxBody.createFixture(boxPoly, 1);

						boxes.add(boxBody);
		}

				boxPoly.dispose();
	}

	@Override
	public void render () {
										long start = TimeUtils.nanoTime();
		world.step(Gdx.graphics.getDeltaTime(), 8, 3);
		float updateTime = (TimeUtils.nanoTime() - start) / 1000000000.0f;

						Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT);
		camera.update();

				renderBox(groundBody, 50, 1);

										batch.getProjectionMatrix().set(camera.combined);
		batch.begin();
		for (int i = 0; i < boxes.size(); i++) {
			Body box = boxes.get(i);
			Vector2 position = box.getPosition(); 			float angle = MathUtils.radiansToDegrees * box.getAngle(); 			batch.draw(textureRegion, position.x - 1, position.y - 1, 				1f, 1f, 				2, 2, 				1, 1, 				angle); 		}
		batch.end();

												debugRenderer.render(world, camera.combined);

				renderer.setProjectionMatrix(camera.combined);
		renderer.begin(ShapeType.Point);
		renderer.setColor(0, 1, 0, 1);
		for (int i = 0; i < world.getContactCount(); i++) {
			Contact contact = world.getContactList().get(i);
						if (contact.isTouching()) {
																WorldManifold manifold = contact.getWorldManifold();
				int numContactPoints = manifold.getNumberOfContactPoints();
				for (int j = 0; j < numContactPoints; j++) {
					Vector2 point = manifold.getPoints()[j];
					renderer.point(point.x, point.y, 0);
				}
			}
		}
		renderer.end();

								batch.getProjectionMatrix().setToOrtho2D(0, 0, Gdx.graphics.getWidth(), Gdx.graphics.getHeight());
		batch.begin();
		font.draw(batch, "fps: " + Gdx.graphics.getFramesPerSecond() + " update time: " + updateTime, 0, 20);
		batch.end();
	}

	Matrix4 transform = new Matrix4();

	private void renderBox (Body body, float halfWidth, float halfHeight) {
				Vector2 pos = body.getWorldCenter();
		float angle = body.getAngle();

				transform.setToTranslation(pos.x, pos.y, 0);
		transform.rotate(0, 0, 1, (float)Math.toDegrees(angle));

				renderer.begin(ShapeType.Line);
		renderer.setTransformMatrix(transform);
		renderer.setColor(1, 1, 1, 1);
		renderer.rect(-halfWidth, -halfHeight, halfWidth * 2, halfHeight * 2);
		renderer.end();
	}

	
	Vector3 testPoint = new Vector3();
	QueryCallback callback = new QueryCallback() {
		@Override
		public boolean reportFixture (Fixture fixture) {
									if (fixture.getBody() == groundBody) return true;

									if (fixture.testPoint(testPoint.x, testPoint.y)) {
				hitBody = fixture.getBody();
				return false;
			} else
				return true;
		}
	};

	@Override
	public boolean touchDown (int x, int y, int pointer, int newParam) {
				testPoint.set(x, y, 0);
		camera.unproject(testPoint);

						hitBody = null;
		world.QueryAABB(callback, testPoint.x - 0.1f, testPoint.y - 0.1f, testPoint.x + 0.1f, testPoint.y + 0.1f);

						if (hitBody != null) {
			MouseJointDef def = new MouseJointDef();
			def.bodyA = groundBody;
			def.bodyB = hitBody;
			def.collideConnected = true;
			def.target.set(testPoint.x, testPoint.y);
			def.maxForce = 1000.0f * hitBody.getMass();

			mouseJoint = (MouseJoint)world.createJoint(def);
			hitBody.setAwake(true);
		} else {
			for (Body box : boxes)
				world.destroyBody(box);
			boxes.clear();
			createBoxes();
		}

		return false;
	}

	
	Vector2 target = new Vector2();

	@Override
	public boolean touchDragged (int x, int y, int pointer) {
								if (mouseJoint != null) {
			camera.unproject(testPoint.set(x, y, 0));
			mouseJoint.setTarget(target.set(testPoint.x, testPoint.y));
		}
		return false;
	}

	@Override
	public boolean touchUp (int x, int y, int pointer, int button) {
				if (mouseJoint != null) {
			world.destroyJoint(mouseJoint);
			mouseJoint = null;
		}
		return false;
	}

	@Override
	public void dispose () {
		world.dispose();
		renderer.dispose();
		debugRenderer.dispose();
		font.dispose();
		textureRegion.getTexture().dispose();
	}
}
