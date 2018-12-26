

package com.badlogic.gdx.tests;

import java.util.Comparator;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.Sprite;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;
import com.badlogic.gdx.graphics.g2d.TextureRegion;
import com.badlogic.gdx.math.MathUtils;
import com.badlogic.gdx.tests.utils.GdxTest;
import com.badlogic.gdx.utils.Array;


public class SortedSpriteTest extends GdxTest {
	
	public class MySprite extends Sprite {
		public float z;

		public MySprite () {
			super();
		}

		public MySprite (Sprite sprite) {
			super(sprite);
		}

		public MySprite (Texture texture, int srcX, int srcY, int srcWidth, int srcHeight) {
			super(texture, srcX, srcY, srcWidth, srcHeight);
		}

		public MySprite (Texture texture, int srcWidth, int srcHeight) {
			super(texture, srcWidth, srcHeight);
		}

		public MySprite (Texture texture) {
			super(texture);
		}

		public MySprite (TextureRegion region, int srcX, int srcY, int srcWidth, int srcHeight) {
			super(region, srcX, srcY, srcWidth, srcHeight);
		}

		public MySprite (TextureRegion region) {
			super(region);
		}
	}

	
	public class MySpriteComparator implements Comparator<MySprite> {
		@Override
		public int compare (MySprite sprite1, MySprite sprite2) {
			return (sprite2.z - sprite1.z) > 0 ? 1 : -1;
		}
	}

	
	SpriteBatch batch;
	
	Texture texture;
	
	Array<MySprite> sprites = new Array<MySprite>();
	
	MySpriteComparator comparator = new MySpriteComparator();

	@Override
	public void create () {
				batch = new SpriteBatch();

						texture = new Texture("data/badlogicsmall.jpg");

								for (int i = 0; i < 100; i++) {
						MySprite sprite = new MySprite(texture);
			sprite.setPosition(MathUtils.random() * Gdx.graphics.getWidth(), MathUtils.random() * Gdx.graphics.getHeight());

						sprite.z = MathUtils.random();

									sprite.setColor(sprite.z, 0, 0, 1);

						sprites.add(sprite);
		}
	}

	@Override
	public void render () {
		Gdx.gl.glClearColor(0.2f, 0.2f, 0.2f, 1);
		Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT);

						sprites.sort(comparator);

				batch.begin();
		for (MySprite sprite : sprites) {
			sprite.draw(batch);
		}
		batch.end();
	}

	@Override
	public void dispose () {
		batch.dispose();
		texture.dispose();
	}
}
