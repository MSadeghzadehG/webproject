

package com.badlogic.gdx.tests;

import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.Pixmap;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.BitmapFont;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;
import com.badlogic.gdx.graphics.g2d.TextureRegion;
import com.badlogic.gdx.math.MathUtils;
import com.badlogic.gdx.tests.utils.GdxTest;
import com.badlogic.gdx.utils.StreamUtils;

public class TextureDownloadTest extends GdxTest {
	TextureRegion image;
	BitmapFont font;
	SpriteBatch batch;

	@Override
	public void create () {
		new Thread(new Runnable() {
			
			private int download (byte[] out, String url) {
				InputStream in = null;
				try {
					HttpURLConnection conn = null;
					conn = (HttpURLConnection)new URL(url).openConnection();
					conn.setDoInput(true);
					conn.setDoOutput(false);
					conn.setUseCaches(true);
					conn.connect();
					in = conn.getInputStream();
					int readBytes = 0;
					while (true) {
						int length = in.read(out, readBytes, out.length - readBytes);
						if (length == -1) break;
						readBytes += length;
					}
					return readBytes;
				} catch (Exception ex) {
					return 0;
				} finally {
					StreamUtils.closeQuietly(in);
				}
			}

			@Override
			public void run () {
				byte[] bytes = new byte[200 * 1024]; 				int numBytes = download(bytes, "http:				if (numBytes != 0) {
										Pixmap pixmap = new Pixmap(bytes, 0, numBytes);
					final int originalWidth = pixmap.getWidth();
					final int originalHeight = pixmap.getHeight();
					int width = MathUtils.nextPowerOfTwo(pixmap.getWidth());
					int height = MathUtils.nextPowerOfTwo(pixmap.getHeight());
					final Pixmap potPixmap = new Pixmap(width, height, pixmap.getFormat());
					potPixmap.drawPixmap(pixmap, 0, 0, 0, 0, pixmap.getWidth(), pixmap.getHeight());
					pixmap.dispose();
					Gdx.app.postRunnable(new Runnable() {
						@Override
						public void run () {
							image = new TextureRegion(new Texture(potPixmap), 0, 0, originalWidth, originalHeight);
						}
					});
				}
			}
		}).start();

		font = new BitmapFont();
		batch = new SpriteBatch();
	}

	@Override
	public void render () {
		Gdx.gl.glClearColor(0.2f, 0.2f, 0.2f, 1);
		Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT);

		if (image != null) {
			batch.begin();
			batch.draw(image, 100, 100);
			batch.end();
		} else {
			batch.begin();
			font.draw(batch, "Downloading...", 100, 100);
			batch.end();
		}
	}
}
