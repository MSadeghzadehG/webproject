

package com.badlogic.gdx.tests;

import java.nio.IntBuffer;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;
import com.badlogic.gdx.tests.utils.GdxTest;

public class SpriteBatchRotationTest extends GdxTest {
	SpriteBatch spriteBatch;
	Texture texture;
		float angle = 0;
	float scale = 1;
	float vScale = 1;
	IntBuffer pixelBuffer;

	@Override
	public void render () {
		Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT);
		spriteBatch.begin();
		spriteBatch.draw(texture, 16, 10, 16, 16, 32, 32, 1, 1, 0, 0, 0, texture.getWidth(), texture.getHeight(), false, false);
		spriteBatch.draw(texture, 64, 10, 32, 32, 0, 0, texture.getWidth(), texture.getHeight(), false, false);
		spriteBatch.draw(texture, 112, 10, 0, 0, texture.getWidth(), texture.getHeight());

		spriteBatch.draw(texture, 16, 58, 16, 16, 32, 32, 1, 1, angle, 0, 0, texture.getWidth(), texture.getHeight(), false, false);
		spriteBatch.draw(texture, 64, 58, 16, 16, 32, 32, scale, scale, 0, 0, 0, texture.getWidth(), texture.getHeight(), false,
			false);
		spriteBatch.draw(texture, 112, 58, 16, 16, 32, 32, scale, scale, angle, 0, 0, texture.getWidth(), texture.getHeight(),
			false, false);
		spriteBatch.draw(texture, 160, 58, 0, 0, 32, 32, scale, scale, angle, 0, 0, texture.getWidth(), texture.getHeight(), false,
			false);

		spriteBatch.end();
		angle += 20 * Gdx.graphics.getDeltaTime();
		scale += vScale * Gdx.graphics.getDeltaTime();
		if (scale > 2) {
			vScale = -vScale;
			scale = 2;
		}
		if (scale < 0) {
			vScale = -vScale;
			scale = 0;
		}

	}

	@Override
	public void create () {
		spriteBatch = new SpriteBatch();
		texture = new Texture(Gdx.files.internal("data/test.png"));
	}
}
