

package com.badlogic.gdx.graphics;

import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.g2d.Gdx2DPixmap;
import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.GdxRuntimeException;

import java.io.IOException;
import java.nio.ByteBuffer;


public class Pixmap implements Disposable {
	
	public enum Format {
		Alpha, Intensity, LuminanceAlpha, RGB565, RGBA4444, RGB888, RGBA8888;

		public static int toGdx2DPixmapFormat (Format format) {
			if (format == Alpha) return Gdx2DPixmap.GDX2D_FORMAT_ALPHA;
			if (format == Intensity) return Gdx2DPixmap.GDX2D_FORMAT_ALPHA;
			if (format == LuminanceAlpha) return Gdx2DPixmap.GDX2D_FORMAT_LUMINANCE_ALPHA;
			if (format == RGB565) return Gdx2DPixmap.GDX2D_FORMAT_RGB565;
			if (format == RGBA4444) return Gdx2DPixmap.GDX2D_FORMAT_RGBA4444;
			if (format == RGB888) return Gdx2DPixmap.GDX2D_FORMAT_RGB888;
			if (format == RGBA8888) return Gdx2DPixmap.GDX2D_FORMAT_RGBA8888;
			throw new GdxRuntimeException("Unknown Format: " + format);
		}

		public static Format fromGdx2DPixmapFormat (int format) {
			if (format == Gdx2DPixmap.GDX2D_FORMAT_ALPHA) return Alpha;
			if (format == Gdx2DPixmap.GDX2D_FORMAT_LUMINANCE_ALPHA) return LuminanceAlpha;
			if (format == Gdx2DPixmap.GDX2D_FORMAT_RGB565) return RGB565;
			if (format == Gdx2DPixmap.GDX2D_FORMAT_RGBA4444) return RGBA4444;
			if (format == Gdx2DPixmap.GDX2D_FORMAT_RGB888) return RGB888;
			if (format == Gdx2DPixmap.GDX2D_FORMAT_RGBA8888) return RGBA8888;
			throw new GdxRuntimeException("Unknown Gdx2DPixmap Format: " + format);
		}
		
		public static int toGlFormat (Format format) {
			return Gdx2DPixmap.toGlFormat(toGdx2DPixmapFormat(format));
		}
		
		public static int toGlType (Format format) {
			return Gdx2DPixmap.toGlType(toGdx2DPixmapFormat(format));
		}
	}

	
	public enum Blending {
		None, SourceOver
	}

	
	public enum Filter {
		NearestNeighbour, BiLinear
	}

	private Blending blending = Blending.SourceOver;
	private Filter filter = Filter.BiLinear;

	final Gdx2DPixmap pixmap;
	int color = 0;

	private boolean disposed;

	
	public void setBlending (Blending blending) {
		this.blending = blending;
		pixmap.setBlend(blending == Blending.None ? 0 : 1);
	}

	
	public void setFilter (Filter filter) {
		this.filter = filter;
		pixmap.setScale(filter == Filter.NearestNeighbour ? Gdx2DPixmap.GDX2D_SCALE_NEAREST : Gdx2DPixmap.GDX2D_SCALE_LINEAR);
	}

	
	public Pixmap (int width, int height, Format format) {
		pixmap = new Gdx2DPixmap(width, height, Format.toGdx2DPixmapFormat(format));
		setColor(0, 0, 0, 0);
		fill();
	}

	
	public Pixmap (byte[] encodedData, int offset, int len) {
		try {
			pixmap = new Gdx2DPixmap(encodedData, offset, len, 0);
		} catch (IOException e) {
			throw new GdxRuntimeException("Couldn't load pixmap from image data", e);
		}
	}

	
	public Pixmap (FileHandle file) {
		try {
			byte[] bytes = file.readBytes();
			pixmap = new Gdx2DPixmap(bytes, 0, bytes.length, 0);
		} catch (Exception e) {
			throw new GdxRuntimeException("Couldn't load file: " + file, e);
		}
	}

	
	public Pixmap (Gdx2DPixmap pixmap) {
		this.pixmap = pixmap;
	}

	
	public void setColor (int color) {
		this.color = color;
	}

	
	public void setColor (float r, float g, float b, float a) {
		color = Color.rgba8888(r, g, b, a);
	}

	
	public void setColor (Color color) {
		this.color = Color.rgba8888(color.r, color.g, color.b, color.a);
	}

	
	public void fill () {
		pixmap.clear(color);
	}


	
	public void drawLine (int x, int y, int x2, int y2) {
		pixmap.drawLine(x, y, x2, y2, color);
	}

	
	public void drawRectangle (int x, int y, int width, int height) {
		pixmap.drawRect(x, y, width, height, color);
	}

	
	public void drawPixmap (Pixmap pixmap, int x, int y) {
		drawPixmap(pixmap, x, y, 0, 0, pixmap.getWidth(), pixmap.getHeight());
	}

	
	public void drawPixmap (Pixmap pixmap, int x, int y, int srcx, int srcy, int srcWidth, int srcHeight) {
		this.pixmap.drawPixmap(pixmap.pixmap, srcx, srcy, x, y, srcWidth, srcHeight);
	}

	
	public void drawPixmap (Pixmap pixmap, int srcx, int srcy, int srcWidth, int srcHeight, int dstx, int dsty, int dstWidth,
		int dstHeight) {
		this.pixmap.drawPixmap(pixmap.pixmap, srcx, srcy, srcWidth, srcHeight, dstx, dsty, dstWidth, dstHeight);
	}

	
	public void fillRectangle (int x, int y, int width, int height) {
		pixmap.fillRect(x, y, width, height, color);
	}

	
	public void drawCircle (int x, int y, int radius) {
		pixmap.drawCircle(x, y, radius, color);
	}

	
	public void fillCircle (int x, int y, int radius) {
		pixmap.fillCircle(x, y, radius, color);
	}

	
	public void fillTriangle (int x1, int y1, int x2, int y2, int x3, int y3) {
		pixmap.fillTriangle(x1, y1, x2, y2, x3, y3, color);
	}

	
	public int getPixel (int x, int y) {
		return pixmap.getPixel(x, y);
	}

	
	public int getWidth () {
		return pixmap.getWidth();
	}

	
	public int getHeight () {
		return pixmap.getHeight();
	}

	
	public void dispose () {
		if (disposed) throw new GdxRuntimeException("Pixmap already disposed!");
		pixmap.dispose();
		disposed = true;
	}

	
	public void drawPixel (int x, int y) {
		pixmap.setPixel(x, y, color);
	}

	
	public void drawPixel (int x, int y, int color) {
		pixmap.setPixel(x, y, color);
	}

	
	public int getGLFormat () {
		return pixmap.getGLFormat();
	}

	
	public int getGLInternalFormat () {
		return pixmap.getGLInternalFormat();
	}

	
	public int getGLType () {
		return pixmap.getGLType();
	}

	
	public ByteBuffer getPixels () {
		if (disposed) throw new GdxRuntimeException("Pixmap already disposed");
		return pixmap.getPixels();
	}

	
	public Format getFormat () {
		return Format.fromGdx2DPixmapFormat(pixmap.getFormat());
	}

	
	public Blending getBlending () {
		return blending;
	}
	
	
	public Filter getFilter (){
		return filter;
	}
}
