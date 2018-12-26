

package com.badlogic.gdx.graphics.g2d.freetype;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;

import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.Color;
import com.badlogic.gdx.graphics.Pixmap;
import com.badlogic.gdx.graphics.Pixmap.Blending;
import com.badlogic.gdx.graphics.Pixmap.Format;
import com.badlogic.gdx.utils.BufferUtils;
import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.LongMap;
import com.badlogic.gdx.utils.SharedLibraryLoader;

public class FreeType {
		
	
	
	static native int getLastErrorCode(); 
	
	private static class Pointer {
		long address;
		
		Pointer(long address) {
			this.address = address;
		}
	}
	
	public static class Library extends Pointer implements Disposable {
		LongMap<ByteBuffer> fontData = new LongMap<ByteBuffer>();
		
		Library (long address) {
			super(address);
		}

		@Override
		public void dispose () {
			doneFreeType(address);
			for(ByteBuffer buffer: fontData.values()) {
				BufferUtils.disposeUnsafeByteBuffer(buffer);
			}
		}

		private static native void doneFreeType(long library); 

		public Face newFace(FileHandle font, int faceIndex) {
			byte[] data = font.readBytes();
			return newMemoryFace(data, data.length, faceIndex);
		}

		public Face newMemoryFace(byte[] data, int dataSize, int faceIndex) {
			ByteBuffer buffer = BufferUtils.newUnsafeByteBuffer(data.length);
			BufferUtils.copy(data, 0, buffer, data.length);
			return newMemoryFace(buffer, faceIndex);
		}

		public Face newMemoryFace(ByteBuffer buffer, int faceIndex) {
			long face = newMemoryFace(address, buffer, buffer.remaining(), faceIndex);
			if(face == 0) {
				BufferUtils.disposeUnsafeByteBuffer(buffer);
				throw new GdxRuntimeException("Couldn't load font, FreeType error code: " + getLastErrorCode());
			}
			else {
				fontData.put(face, buffer);
				return new Face(face, this);
			}
		}

		private static native long newMemoryFace(long library, ByteBuffer data, int dataSize, int faceIndex); 

		public Stroker createStroker() {
			long stroker = strokerNew(address);
			if(stroker == 0) throw new GdxRuntimeException("Couldn't create FreeType stroker, FreeType error code: " + getLastErrorCode());
			return new Stroker(stroker);
		}

		private static native long strokerNew(long library); 
	}
	
	public static class Face extends Pointer implements Disposable {
		Library library;
		
		public Face (long address, Library library) {
			super(address);
			this.library = library;
		}
		
		@Override
		public void dispose() {
			doneFace(address);
			ByteBuffer buffer = library.fontData.get(address);
			if(buffer != null) {
				library.fontData.remove(address);
				BufferUtils.disposeUnsafeByteBuffer(buffer);
			}
		}

		private static native void doneFace(long face); 

		public int getFaceFlags() {
			return getFaceFlags(address);
		}
		
		private static native int getFaceFlags(long face); 
		
		public int getStyleFlags() {
			return getStyleFlags(address);
		}
		
		private static native int getStyleFlags(long face); 
		
		public int getNumGlyphs() {
			return getNumGlyphs(address);
		}
		
		private static native int getNumGlyphs(long face); 
		
		public int getAscender() {
			return getAscender(address);
		}
		
		private static native int getAscender(long face); 
		
		public int getDescender() {
			return getDescender(address);
		}
		
		private static native int getDescender(long face); 
		
		public int getHeight() {
			return getHeight(address);
		}
		
		private static native int getHeight(long face); 
		
		public int getMaxAdvanceWidth() {
			return getMaxAdvanceWidth(address);
		}
		
		private static native int getMaxAdvanceWidth(long face); 
		
		public int getMaxAdvanceHeight() {
			return getMaxAdvanceHeight(address);
		}
		
		private static native int getMaxAdvanceHeight(long face); 
		
		public int getUnderlinePosition() {
			return getUnderlinePosition(address);
		}
		
		private static native int getUnderlinePosition(long face); 
		
		public int getUnderlineThickness() {
			return getUnderlineThickness(address);
		}
		
		private static native int getUnderlineThickness(long face); 
		
		public boolean selectSize(int strikeIndex) {
			return selectSize(address, strikeIndex);
		}

		private static native boolean selectSize(long face, int strike_index); 

		public boolean setCharSize(int charWidth, int charHeight, int horzResolution, int vertResolution) {
			return setCharSize(address, charWidth, charHeight, horzResolution, vertResolution);
		}

		private static native boolean setCharSize(long face, int charWidth, int charHeight, int horzResolution, int vertResolution); 

		public boolean setPixelSizes(int pixelWidth, int pixelHeight) {
			return setPixelSizes(address, pixelWidth, pixelHeight);
		}

		private static native boolean setPixelSizes(long face, int pixelWidth, int pixelHeight); 

		public boolean loadGlyph(int glyphIndex, int loadFlags) {
			return loadGlyph(address, glyphIndex, loadFlags);
		}

		private static native boolean loadGlyph(long face, int glyphIndex, int loadFlags); 

		public boolean loadChar(int charCode, int loadFlags) {
			return loadChar(address, charCode, loadFlags);
		}

		private static native boolean loadChar(long face, int charCode, int loadFlags); 

		public GlyphSlot getGlyph() {
			return new GlyphSlot(getGlyph(address));
		}
		
		private static native long getGlyph(long face); 
		
		public Size getSize() {
			return new Size(getSize(address));
		}
		
		private static native long getSize(long face); 

		public boolean hasKerning() {
			return hasKerning(address);
		}

		private static native boolean hasKerning(long face); 

		public int getKerning(int leftGlyph, int rightGlyph, int kernMode) {
			return getKerning(address, leftGlyph, rightGlyph, kernMode);
		}

		private static native int getKerning(long face, int leftGlyph, int rightGlyph, int kernMode); 

		public int getCharIndex(int charCode) {
			return getCharIndex(address, charCode);
		}

		private static native int getCharIndex(long face, int charCode); 

	}
	
	public static class Size extends Pointer {
		Size (long address) {
			super(address);
		}
		
		public SizeMetrics getMetrics() {
			return new SizeMetrics(getMetrics(address));
		}
		
		private static native long getMetrics(long address); 
	}
	
	public static class SizeMetrics extends Pointer {
		SizeMetrics (long address) {
			super(address);
		}
		
		public int getXppem() {
			return getXppem(address);
		}
		
		private static native int getXppem(long metrics); 
		
		public int getYppem() {
			return getYppem(address);
		}
		
		private static native int getYppem(long metrics); 
		
		public int getXScale() {
			return getXscale(address);
		}
		
		private static native int getXscale(long metrics); 
		
		public int getYscale() {
			return getYscale(address);
		}
		
		private static native int getYscale(long metrics); 
		
		public int getAscender() {
			return getAscender(address);
		}
		
		private static native int getAscender(long metrics); 
		
		public int getDescender() {
			return getDescender(address);
		}
		
		private static native int getDescender(long metrics); 
		
		public int getHeight() {
			return getHeight(address);
		}
		
		private static native int getHeight(long metrics); 
		
		public int getMaxAdvance() {
			return getMaxAdvance(address);
		}
		
		private static native int getMaxAdvance(long metrics); 
	}
	
	public static class GlyphSlot extends Pointer {
		GlyphSlot (long address) {
			super(address);
		}
		
		public GlyphMetrics getMetrics() {
			return new GlyphMetrics(getMetrics(address));
		}		
		
		private static native long getMetrics(long slot); 
		
		public int getLinearHoriAdvance() {
			return getLinearHoriAdvance(address);
		}
		
		private static native int getLinearHoriAdvance(long slot); 
		
		public int getLinearVertAdvance() {
			return getLinearVertAdvance(address);
		}
		
		private static native int getLinearVertAdvance(long slot); 
		
		public int getAdvanceX() {
			return getAdvanceX(address);
		}
		
		private static native int getAdvanceX(long slot); 
		
		public int getAdvanceY() {
			return getAdvanceY(address);
		}
		
		private static native int getAdvanceY(long slot); 
		
		public int getFormat() {
			return getFormat(address);
		}
		
		private static native int getFormat(long slot); 
		
		public Bitmap getBitmap() {
			return new Bitmap(getBitmap(address));
		}
		
		private static native long getBitmap(long slot); 
		
		public int getBitmapLeft() {
			return getBitmapLeft(address);
		}
		
		private static native int getBitmapLeft(long slot); 
		
		public int getBitmapTop() {
			return getBitmapTop(address);
		}
		
		private static native int getBitmapTop(long slot); 

		public boolean renderGlyph(int renderMode) {
			return renderGlyph(address, renderMode);
		}

		private static native boolean renderGlyph(long slot, int renderMode); 

		public Glyph getGlyph() {
			long glyph = getGlyph(address);
			if(glyph == 0) throw new GdxRuntimeException("Couldn't get glyph, FreeType error code: " + getLastErrorCode());
			return new Glyph(glyph);
		}

		private static native long getGlyph(long glyphSlot); 
	}
	
	public static class Glyph extends Pointer implements Disposable {
		private boolean rendered;

		Glyph (long address) {
			super(address);
		}

		@Override
		public void dispose () {
			done(address);
		}

		private static native void done(long glyph); 

		public void strokeBorder(Stroker stroker, boolean inside) {
			address = strokeBorder(address, stroker.address, inside);
		}

		private static native long strokeBorder(long glyph, long stroker, boolean inside); 

		public void toBitmap(int renderMode) {
			long bitmap = toBitmap(address, renderMode);
			if (bitmap == 0) throw new GdxRuntimeException("Couldn't render glyph, FreeType error code: " + getLastErrorCode());
			address = bitmap;
			rendered = true;
		}

		private static native long toBitmap(long glyph, int renderMode); 

		public Bitmap getBitmap() {
			if (!rendered) {
				throw new GdxRuntimeException("Glyph is not yet rendered");
			}
			return new Bitmap(getBitmap(address));
		}

		private static native long getBitmap(long glyph); 

		public int getLeft() {
			if (!rendered) {
				throw new GdxRuntimeException("Glyph is not yet rendered");
			}
			return getLeft(address);
		}

		private static native int getLeft(long glyph); 

		public int getTop() {
			if (!rendered) {
				throw new GdxRuntimeException("Glyph is not yet rendered");
			}
			return getTop(address);
		}

		private static native int getTop(long glyph); 

	}

	public static class Bitmap extends Pointer {
		Bitmap (long address) {
			super(address);
		}
		
		public int getRows() {
			return getRows(address);
		}
		
		private static native int getRows(long bitmap); 
		
		public int getWidth() {
			return getWidth(address);
		}
		
		private static native int getWidth(long bitmap); 
		
		public int getPitch() {
			return getPitch(address);
		}
		
		private static native int getPitch(long bitmap); 
		
		public ByteBuffer getBuffer() {
			if (getRows() == 0)
																								return BufferUtils.newByteBuffer(1);
			return getBuffer(address);
		}

		private static native ByteBuffer getBuffer(long bitmap); 

				public Pixmap getPixmap (Format format, Color color, float gamma) {
			int width = getWidth(), rows = getRows();
			ByteBuffer src = getBuffer();
			Pixmap pixmap;
			int pixelMode = getPixelMode();
			int rowBytes = Math.abs(getPitch()); 			if (color == Color.WHITE && pixelMode == FT_PIXEL_MODE_GRAY && rowBytes == width && gamma == 1) {
				pixmap = new Pixmap(width, rows, Format.Alpha);
				BufferUtils.copy(src, pixmap.getPixels(), pixmap.getPixels().capacity());
			} else {
				pixmap = new Pixmap(width, rows, Format.RGBA8888);
				int rgba = Color.rgba8888(color);
				byte[] srcRow = new byte[rowBytes];
				int[] dstRow = new int[width];
				IntBuffer dst = pixmap.getPixels().asIntBuffer();
				if (pixelMode == FT_PIXEL_MODE_MONO) {
										for (int y = 0; y < rows; y++) {
						src.get(srcRow);
						for (int i = 0, x = 0; x < width; i++, x += 8) {
							byte b = srcRow[i];
							for (int ii = 0, n = Math.min(8, width - x); ii < n; ii++) {
								if ((b & (1 << (7 - ii))) != 0)
									dstRow[x + ii] = rgba;
								else
									dstRow[x + ii] = 0;
							}
						}
						dst.put(dstRow);
					}
				} else {
										int rgb = rgba & 0xffffff00;
					int a = rgba & 0xff;
					for (int y = 0; y < rows; y++) {
						src.get(srcRow);
						for (int x = 0; x < width; x++) {
																												int alpha = srcRow[x] & 0xff;
							if (alpha == 0)
								dstRow[x] = rgb;
							else if (alpha == 255)
								dstRow[x] = rgb | a;
							else
								dstRow[x] = rgb | (int)(a * (float)Math.pow(alpha / 255f, gamma)); 						}
						dst.put(dstRow);
					}
				}
			}

			Pixmap converted = pixmap;
			if (format != pixmap.getFormat()) {
				converted = new Pixmap(pixmap.getWidth(), pixmap.getHeight(), format);
				converted.setBlending(Blending.None);
				converted.drawPixmap(pixmap, 0, 0);
				pixmap.dispose();
			}
			return converted;
		}
		
		public int getNumGray() {
			return getNumGray(address);
		}
		
		private static native int getNumGray(long bitmap); 
		
		public int getPixelMode() {
			return getPixelMode(address);
		}
		
		private static native int getPixelMode(long bitmap); 
	}
	
	public static class GlyphMetrics extends Pointer {
		GlyphMetrics (long address) {
			super(address);
		}
		
		public int getWidth() {
			return getWidth(address);
		}
		
		private static native int getWidth(long metrics); 
		
		public int getHeight() {
			return getHeight(address);
		}
		
		private static native int getHeight(long metrics); 
		
		public int getHoriBearingX() {
			return getHoriBearingX(address);
		}
		
		private static native int getHoriBearingX(long metrics); 
		
		public int getHoriBearingY() {
			return getHoriBearingY(address);
		}
		
		private static native int getHoriBearingY(long metrics); 
		
		public int getHoriAdvance() {
			return getHoriAdvance(address);
		}
		
		private static native int getHoriAdvance(long metrics); 
	
		public int getVertBearingX() {
			return getVertBearingX(address);
		}
		
		private static native int getVertBearingX(long metrics); 
		
		public int getVertBearingY() {
			return getVertBearingY(address);
		}
	
		private static native int getVertBearingY(long metrics); 
		
		public int getVertAdvance() {
			return getVertAdvance(address);
		}
	
		private static native int getVertAdvance(long metrics); 
	}

	public static class Stroker extends Pointer implements Disposable {
		Stroker(long address) {
			super(address);
		}

		public void set(int radius, int lineCap, int lineJoin, int miterLimit) {
			set(address, radius, lineCap, lineJoin, miterLimit);
		}

		private static native void set(long stroker, int radius, int lineCap, int lineJoin, int miterLimit); 

		@Override
		public void dispose() {
			done(address);
		}

		private static native void done(long stroker); 
	}

   public static int FT_PIXEL_MODE_NONE = 0;
   public static int FT_PIXEL_MODE_MONO = 1;
   public static int FT_PIXEL_MODE_GRAY = 2;
   public static int FT_PIXEL_MODE_GRAY2 = 3;
   public static int FT_PIXEL_MODE_GRAY4 = 4;
   public static int FT_PIXEL_MODE_LCD = 5;
   public static int FT_PIXEL_MODE_LCD_V = 6;
	
	private static int encode (char a, char b, char c, char d) {
		return (a << 24) | (b << 16) | (c << 8) | d;
	}

	public static int FT_ENCODING_NONE = 0;
	public static int FT_ENCODING_MS_SYMBOL = encode('s', 'y', 'm', 'b');
	public static int FT_ENCODING_UNICODE = encode('u', 'n', 'i', 'c');
	public static int FT_ENCODING_SJIS = encode('s', 'j', 'i', 's');
	public static int FT_ENCODING_GB2312 = encode('g', 'b', ' ', ' ');
	public static int FT_ENCODING_BIG5 = encode('b', 'i', 'g', '5');
	public static int FT_ENCODING_WANSUNG = encode('w', 'a', 'n', 's');
	public static int FT_ENCODING_JOHAB = encode('j', 'o', 'h', 'a');
	public static int FT_ENCODING_ADOBE_STANDARD = encode('A', 'D', 'O', 'B');
	public static int FT_ENCODING_ADOBE_EXPERT = encode('A', 'D', 'B', 'E');
	public static int FT_ENCODING_ADOBE_CUSTOM = encode('A', 'D', 'B', 'C');
	public static int FT_ENCODING_ADOBE_LATIN_1 = encode('l', 'a', 't', '1');
	public static int FT_ENCODING_OLD_LATIN_2 = encode('l', 'a', 't', '2');
	public static int FT_ENCODING_APPLE_ROMAN = encode('a', 'r', 'm', 'n');
	
	public static int FT_FACE_FLAG_SCALABLE          = ( 1 <<  0 );
	public static int FT_FACE_FLAG_FIXED_SIZES       = ( 1 <<  1 );
	public static int FT_FACE_FLAG_FIXED_WIDTH       = ( 1 <<  2 );
	public static int FT_FACE_FLAG_SFNT              = ( 1 <<  3 );
	public static int FT_FACE_FLAG_HORIZONTAL        = ( 1 <<  4 );
	public static int FT_FACE_FLAG_VERTICAL          = ( 1 <<  5 );
	public static int FT_FACE_FLAG_KERNING           = ( 1 <<  6 );
	public static int FT_FACE_FLAG_FAST_GLYPHS       = ( 1 <<  7 );
	public static int FT_FACE_FLAG_MULTIPLE_MASTERS  = ( 1 <<  8 );
	public static int FT_FACE_FLAG_GLYPH_NAMES       = ( 1 <<  9 );
	public static int FT_FACE_FLAG_EXTERNAL_STREAM   = ( 1 << 10 );
	public static int FT_FACE_FLAG_HINTER            = ( 1 << 11 );
	public static int FT_FACE_FLAG_CID_KEYED         = ( 1 << 12 );
	public static int FT_FACE_FLAG_TRICKY            = ( 1 << 13 );
	
	public static int FT_STYLE_FLAG_ITALIC = ( 1 << 0 );
	public static int FT_STYLE_FLAG_BOLD   = ( 1 << 1 );
	
	public static int FT_LOAD_DEFAULT                      = 0x0;
	public static int FT_LOAD_NO_SCALE                     = 0x1;
	public static int FT_LOAD_NO_HINTING                   = 0x2;
	public static int FT_LOAD_RENDER                       = 0x4;
	public static int FT_LOAD_NO_BITMAP                    = 0x8;
	public static int FT_LOAD_VERTICAL_LAYOUT              = 0x10;
	public static int FT_LOAD_FORCE_AUTOHINT               = 0x20;
	public static int FT_LOAD_CROP_BITMAP                  = 0x40;
	public static int FT_LOAD_PEDANTIC                     = 0x80;
	public static int FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH  = 0x200;
	public static int FT_LOAD_NO_RECURSE                   = 0x400;
	public static int FT_LOAD_IGNORE_TRANSFORM             = 0x800;
	public static int FT_LOAD_MONOCHROME                   = 0x1000;
	public static int FT_LOAD_LINEAR_DESIGN                = 0x2000;
	public static int FT_LOAD_NO_AUTOHINT                  = 0x8000;
	
	public static int FT_LOAD_TARGET_NORMAL                = 0x0;
	public static int FT_LOAD_TARGET_LIGHT                 = 0x10000;
	public static int FT_LOAD_TARGET_MONO                  = 0x20000;
	public static int FT_LOAD_TARGET_LCD                   = 0x30000;
	public static int FT_LOAD_TARGET_LCD_V                 = 0x40000;

   public static int FT_RENDER_MODE_NORMAL = 0;
   public static int FT_RENDER_MODE_LIGHT = 1;
   public static int FT_RENDER_MODE_MONO = 2;
   public static int FT_RENDER_MODE_LCD = 3;
   public static int FT_RENDER_MODE_LCD_V = 4;
   public static int FT_RENDER_MODE_MAX = 5;
   
   public static int FT_KERNING_DEFAULT = 0;
   public static int FT_KERNING_UNFITTED = 1;
   public static int FT_KERNING_UNSCALED = 2;
	
	public static int FT_STROKER_LINECAP_BUTT = 0;
	public static int FT_STROKER_LINECAP_ROUND = 1;
	public static int FT_STROKER_LINECAP_SQUARE = 2;

	public static int FT_STROKER_LINEJOIN_ROUND          = 0;
	public static int FT_STROKER_LINEJOIN_BEVEL          = 1;
	public static int FT_STROKER_LINEJOIN_MITER_VARIABLE = 2;
	public static int FT_STROKER_LINEJOIN_MITER          = FT_STROKER_LINEJOIN_MITER_VARIABLE;
	public static int FT_STROKER_LINEJOIN_MITER_FIXED    = 3;

   public static Library initFreeType() {   	
   	new SharedLibraryLoader().load("gdx-freetype");
   	long address = initFreeTypeJni();
   	if(address == 0) throw new GdxRuntimeException("Couldn't initialize FreeType library, FreeType error code: " + getLastErrorCode());
   	else return new Library(address);
   }
   
	private static native long initFreeTypeJni(); 

	public static int toInt (int value) {
		return ((value + 63) & -64) >> 6;
	}
   
}
