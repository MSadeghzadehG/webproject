

package com.badlogic.gdx.tests;

import java.util.Arrays;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.FPSLogger;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.PerspectiveCamera;
import com.badlogic.gdx.graphics.Pixmap.Format;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;
import com.badlogic.gdx.graphics.g2d.TextureRegion;
import com.badlogic.gdx.graphics.g3d.Model;
import com.badlogic.gdx.graphics.g3d.ModelBatch;
import com.badlogic.gdx.graphics.g3d.ModelInstance;
import com.badlogic.gdx.graphics.g3d.loader.ObjLoader;
import com.badlogic.gdx.graphics.glutils.FrameBuffer;
import com.badlogic.gdx.graphics.glutils.ShaderProgram;
import com.badlogic.gdx.math.Matrix4;
import com.badlogic.gdx.tests.utils.GdxTest;

public class EdgeDetectionTest extends GdxTest {

	FPSLogger logger;
		Model scene;
	ModelInstance sceneInstance;
	ModelBatch modelBatch;
	FrameBuffer fbo;
	PerspectiveCamera cam;
	Matrix4 matrix = new Matrix4();
	float angle = 0;
	TextureRegion fboRegion;
	SpriteBatch batch;
	ShaderProgram batchShader;

	float[] filter = {0, 0.25f, 0, 0.25f, -1f, 0.6f, 0, 0.25f, 0,};

	float[] offsets = new float[18];

	public void create () {
		ShaderProgram.pedantic = false;
		
		batchShader = new ShaderProgram(Gdx.files.internal("data/shaders/batch.vert").readString(), Gdx.files.internal(
			"data/shaders/convolution.frag").readString());
		if (!batchShader.isCompiled()) {
			Gdx.app.log("EdgeDetectionTest", "couldn't compile post-processing shader: " + batchShader.getLog());
		}

		ObjLoader objLoader = new ObjLoader();
		scene = objLoader.loadModel(Gdx.files.internal("data/scene.obj"));
		sceneInstance = new ModelInstance(scene);
		modelBatch = new ModelBatch();
		fbo = new FrameBuffer(Format.RGB565, Gdx.graphics.getWidth(), Gdx.graphics.getHeight(), true);
		cam = new PerspectiveCamera(67, Gdx.graphics.getWidth(), Gdx.graphics.getHeight());
		cam.position.set(0, 0, 10);
		cam.lookAt(0, 0, 0);
		cam.far = 30;
		batch = new SpriteBatch();
		batch.setShader(batchShader);
		fboRegion = new TextureRegion(fbo.getColorBufferTexture());
		fboRegion.flip(false, true);
		logger = new FPSLogger();
		calculateOffsets();
	}

	@Override
	public void dispose () {
		batchShader.dispose();
		scene.dispose();
		fbo.dispose();
		batch.dispose();
	}

	private void calculateOffsets () {
		int idx = 0;
		for (int y = -1; y <= 1; y++) {
			for (int x = -1; x <= 1; x++) {
				offsets[idx++] = x / (float)Gdx.graphics.getWidth();
				offsets[idx++] = y / (float)Gdx.graphics.getHeight();
			}
		}
		System.out.println(Arrays.toString(offsets));
	}

	public void render () {
		angle += 45 * Gdx.graphics.getDeltaTime();
		Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT | GL20.GL_DEPTH_BUFFER_BIT);

		cam.update();
		matrix.setToRotation(0, 1, 0, angle);
		cam.combined.mul(matrix);

		fbo.begin();
		Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT | GL20.GL_DEPTH_BUFFER_BIT);
		Gdx.gl.glEnable(GL20.GL_DEPTH_TEST);
		modelBatch.begin(cam);
		modelBatch.render(sceneInstance);
		modelBatch.end();
		fbo.end();

		batch.begin();
		batch.disableBlending();
		batchShader.setUniformi("u_filterSize", filter.length);
		batchShader.setUniform1fv("u_filter", filter, 0, filter.length);
		batchShader.setUniform2fv("u_offsets", offsets, 0, offsets.length);
		batch.draw(fboRegion, 0, 0, Gdx.graphics.getWidth(), Gdx.graphics.getHeight());
		batch.end();
		logger.log();
	}
}
