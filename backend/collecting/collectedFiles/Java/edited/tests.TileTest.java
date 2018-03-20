

package com.badlogic.gdx.tests;

import java.util.Random;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.OrthographicCamera;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.SpriteCache;
import com.badlogic.gdx.tests.utils.GdxTest;
import com.badlogic.gdx.tests.utils.OrthoCamController;
import com.badlogic.gdx.utils.TimeUtils;

public class TileTest extends GdxTest {
	static final int LAYERS = 5;
	static final int BLOCK_TILES = 25;
	static final int WIDTH = 15;
	static final int HEIGHT = 10;
	static final int TILES_PER_LAYER = WIDTH * HEIGHT;

	SpriteCache[] caches = new SpriteCache[LAYERS];
	Texture texture;
	int[] layers = new int[LAYERS];
	OrthographicCamera cam;
	OrthoCamController camController;
	long startTime = TimeUtils.nanoTime();

	@Override
	public void create () {
		cam = new OrthographicCamera(480, 320);
		cam.position.set(WIDTH * 32 / 2, HEIGHT * 32 / 2, 0);
		camController = new OrthoCamController(cam);
		Gdx.input.setInputProcessor(camController);

		texture = new Texture(Gdx.files.internal("data/tiles.png"));

		Random rand = new Random();
		for (int i = 0; i < LAYERS; i++) {
			caches[i] = new SpriteCache();
			SpriteCache cache = caches[i];
			cache.beginCache();
			for (int y = 0; y < HEIGHT; y++) {
				for (int x = 0; x < WIDTH; x++) {
					int tileX = rand.nextInt(5);
					int tileY = rand.nextInt(5);
					cache.add(texture, x << 5, y << 5, 1 + tileX * 33, 1 + tileY * 33, 32, 32);
				}
			}
			layers[i] = cache.endCache();
		}

	}

	@Override
	public void render () {
		Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT);
		cam.update();

		Gdx.gl.glEnable(GL20.GL_BLEND);
		Gdx.gl.glBlendFunc(GL20.GL_SRC_ALPHA, GL20.GL_ONE_MINUS_SRC_ALPHA);
		for (int i = 0; i < LAYERS; i++) {
			SpriteCache cache = caches[i];
			cache.setProjectionMatrix(cam.combined);
			cache.begin();
			for (int j = 0; j < TILES_PER_LAYER; j += BLOCK_TILES) {
				cache.draw(layers[i], j, BLOCK_TILES);
			}
			cache.end();
		}

		if (TimeUtils.nanoTime() - startTime >= 1000000000) {
			Gdx.app.log("TileTest", "fps: " + Gdx.graphics.getFramesPerSecond());
			startTime = TimeUtils.nanoTime();
		}
	}
}
