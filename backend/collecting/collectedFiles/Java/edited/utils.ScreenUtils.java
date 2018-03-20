

package com.badlogic.gdx.utils;

import java.nio.ByteBuffer;
import java.nio.HasArrayBufferView;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.Pixmap;
import com.badlogic.gdx.graphics.Pixmap.Format;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.TextureRegion;
import com.badlogic.gdx.math.MathUtils;
import com.google.gwt.canvas.dom.client.Context2d;
import com.google.gwt.typedarrays.shared.ArrayBufferView;


public final class ScreenUtils {

	
	public static TextureRegion getFrameBufferTexture () {
		final int w = Gdx.graphics.getBackBufferWidth();
		final int h = Gdx.graphics.getBackBufferHeight();
		return getFrameBufferTexture(0, 0, w, h);
	}

	
	public static TextureRegion getFrameBufferTexture (int x, int y, int w, int h) {
		final int potW = MathUtils.nextPowerOfTwo(w);
		final int potH = MathUtils.nextPowerOfTwo(h);

		final Pixmap pixmap = getFrameBufferPixmap(x, y, w, h);
		final Pixmap potPixmap = new Pixmap(potW, potH, Format.RGBA8888);
		potPixmap.drawPixmap(pixmap, 0, 0);
		Texture texture = new Texture(potPixmap);
		TextureRegion textureRegion = new TextureRegion(texture, 0, h, w, -h);
		potPixmap.dispose();
		pixmap.dispose();

		return textureRegion;
	}

	public static Pixmap getFrameBufferPixmap (int x, int y, int w, int h) {
		Gdx.gl.glPixelStorei(GL20.GL_PACK_ALIGNMENT, 1);

		final Pixmap pixmap = new Pixmap(w, h, Format.RGBA8888);
		ByteBuffer pixels = BufferUtils.newByteBuffer(h * w * 4);
		Gdx.gl.glReadPixels(x, y, w, h, GL20.GL_RGBA, GL20.GL_UNSIGNED_BYTE, pixels);
		putPixelsBack(pixmap, pixels);
		return pixmap;
	}

	public static void putPixelsBack (Pixmap pixmap, ByteBuffer pixels) {
		if (pixmap.getWidth() == 0 || pixmap.getHeight() == 0) return;
		putPixelsBack(((HasArrayBufferView)pixels).getTypedArray(), pixmap.getWidth(), pixmap.getHeight(), pixmap.getContext());

	}

	private native static void putPixelsBack (ArrayBufferView pixels, int width, int height, Context2d ctx);

	
	public static byte[] getFrameBufferPixels (boolean flipY) {
		final int w = Gdx.graphics.getBackBufferWidth();
		final int h = Gdx.graphics.getBackBufferHeight();
		return getFrameBufferPixels(0, 0, w, h, flipY);
	}

	
	public static byte[] getFrameBufferPixels (int x, int y, int w, int h, boolean flipY) {
		Gdx.gl.glPixelStorei(GL20.GL_PACK_ALIGNMENT, 1);
		final ByteBuffer pixels = BufferUtils.newByteBuffer(w * h * 4);
		Gdx.gl.glReadPixels(x, y, w, h, GL20.GL_RGBA, GL20.GL_UNSIGNED_BYTE, pixels);
		final int numBytes = w * h * 4;
		byte[] lines = new byte[numBytes];
		if (flipY) {
			final int numBytesPerLine = w * 4;
			for (int i = 0; i < h; i++) {
				pixels.position((h - i - 1) * numBytesPerLine);
				pixels.get(lines, i * numBytesPerLine, numBytesPerLine);
			}
		} else {
			pixels.clear();
			pixels.get(lines);
		}
		return lines;

	}
}
