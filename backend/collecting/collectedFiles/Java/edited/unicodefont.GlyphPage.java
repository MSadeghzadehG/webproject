

package com.badlogic.gdx.tools.hiero.unicodefont;

import java.awt.AlphaComposite;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.font.FontRenderContext;
import java.awt.image.BufferedImage;
import java.awt.image.WritableRaster;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL12;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.Pixmap;
import com.badlogic.gdx.graphics.Pixmap.Format;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.BitmapFont;
import com.badlogic.gdx.graphics.g2d.BitmapFont.BitmapFontData;
import com.badlogic.gdx.tools.hiero.unicodefont.UnicodeFont.RenderType;
import com.badlogic.gdx.tools.hiero.unicodefont.effects.ColorEffect;
import com.badlogic.gdx.tools.hiero.unicodefont.effects.Effect;
import com.badlogic.gdx.utils.Array;


public class GlyphPage {
	private final UnicodeFont unicodeFont;
	private final int pageWidth, pageHeight;
	private final Texture texture;
	private final List<Glyph> pageGlyphs = new ArrayList(32);
	private final List<String> hashes = new ArrayList(32);
	Array<Row> rows = new Array();

	
	GlyphPage (UnicodeFont unicodeFont, int pageWidth, int pageHeight) {
		this.unicodeFont = unicodeFont;
		this.pageWidth = pageWidth;
		this.pageHeight = pageHeight;

		texture = new Texture(pageWidth, pageHeight, Format.RGBA8888);
		rows.add(new Row());
	}

	
	int loadGlyphs (List glyphs, int maxGlyphsToLoad) {
		GL11.glColor4f(1, 1, 1, 1);
		texture.bind();

		int loadedCount = 0;
		for (Iterator iter = glyphs.iterator(); iter.hasNext();) {
			Glyph glyph = (Glyph)iter.next();
			int width = Math.min(MAX_GLYPH_SIZE, glyph.getWidth());
			int height = Math.min(MAX_GLYPH_SIZE, glyph.getHeight());
			if (width == 0 || height == 0)
				pageGlyphs.add(glyph);
			else {
				Row bestRow = null;
								for (int ii = 0, nn = rows.size - 1; ii < nn; ii++) {
					Row row = rows.get(ii);
					if (row.x + width >= pageWidth) continue;
					if (row.y + height >= pageHeight) continue;
					if (height > row.height) continue;
					if (bestRow == null || row.height < bestRow.height) bestRow = row;
				}
				if (bestRow == null) {
										Row row = rows.peek();
					if (row.y + height >= pageHeight) continue;
					if (row.x + width < pageWidth) {
						row.height = Math.max(row.height, height);
						bestRow = row;
					} else if (row.y + row.height + height < pageHeight) {
												bestRow = new Row();
						bestRow.y = row.y + row.height;
						bestRow.height = height;
						rows.add(bestRow);
					}
				}
				if (bestRow == null) continue;

				if (renderGlyph(glyph, bestRow.x, bestRow.y, width, height)) bestRow.x += width;
			}

			iter.remove();
			loadedCount++;
			if (loadedCount == maxGlyphsToLoad) break;

		}

		return loadedCount;
	}

	static class Row {
		int x, y, height;
	}

	
	private boolean renderGlyph (Glyph glyph, int pageX, int pageY, int width, int height) {
		scratchGraphics.setComposite(AlphaComposite.Clear);
		scratchGraphics.fillRect(0, 0, MAX_GLYPH_SIZE, MAX_GLYPH_SIZE);
		scratchGraphics.setComposite(AlphaComposite.SrcOver);

		ByteBuffer glyphPixels = scratchByteBuffer;
		int format;
		if (unicodeFont.getRenderType() == RenderType.FreeType && unicodeFont.bitmapFont != null) {
			BitmapFontData data = unicodeFont.bitmapFont.getData();
			BitmapFont.Glyph g = data.getGlyph((char)glyph.getCodePoint());
			Pixmap fontPixmap = unicodeFont.bitmapFont.getRegions().get(g.page).getTexture().getTextureData().consumePixmap();

			int fontWidth = fontPixmap.getWidth();
			int padTop = unicodeFont.getPaddingTop(), padBottom = unicodeFont.getPaddingBottom();
			int padLeftBytes = unicodeFont.getPaddingLeft() * 4;
			int padXBytes = padLeftBytes + unicodeFont.getPaddingRight() * 4;
			int glyphRowBytes = width * 4, fontRowBytes = g.width * 4;

			ByteBuffer fontPixels = fontPixmap.getPixels();
			byte[] row = new byte[glyphRowBytes];
			glyphPixels.position(0);
			for (int i = 0; i < padTop; i++)
				glyphPixels.put(row);
			glyphPixels.position((height - padBottom) * glyphRowBytes);
			for (int i = 0; i < padBottom; i++)
				glyphPixels.put(row);
			glyphPixels.position(padTop * glyphRowBytes);
			for (int y = 0, n = g.height; y < n; y++) {
				fontPixels.position(((g.srcY + y) * fontWidth + g.srcX) * 4);
				fontPixels.get(row, padLeftBytes, fontRowBytes);
				glyphPixels.put(row);
			}
			fontPixels.position(0);
			glyphPixels.position(height * glyphRowBytes);
			glyphPixels.flip();
			format = GL11.GL_RGBA;
		} else {
						if (unicodeFont.getRenderType() == RenderType.Native) {
				for (Iterator iter = unicodeFont.getEffects().iterator(); iter.hasNext();) {
					Effect effect = (Effect)iter.next();
					if (effect instanceof ColorEffect) scratchGraphics.setColor(((ColorEffect)effect).getColor());
				}
				scratchGraphics.setColor(java.awt.Color.white);
				scratchGraphics.setFont(unicodeFont.getFont());
				scratchGraphics.drawString("" + (char)glyph.getCodePoint(), 0, unicodeFont.getAscent());
			} else if (unicodeFont.getRenderType() == RenderType.Java) {
				scratchGraphics.setColor(java.awt.Color.white);
				for (Iterator iter = unicodeFont.getEffects().iterator(); iter.hasNext();)
					((Effect)iter.next()).draw(scratchImage, scratchGraphics, unicodeFont, glyph);
				glyph.setShape(null); 			}

			width = Math.min(width, texture.getWidth());
			height = Math.min(height, texture.getHeight());

			WritableRaster raster = scratchImage.getRaster();
			int[] row = new int[width];
			for (int y = 0; y < height; y++) {
				raster.getDataElements(0, y, width, 1, row);
				scratchIntBuffer.put(row);
			}
			format = GL12.GL_BGRA;
		}

				String hash = "";
		try {
			MessageDigest md = MessageDigest.getInstance("SHA-256");
			md.update(glyphPixels);
			BigInteger bigInt = new BigInteger(1, md.digest());
			hash = bigInt.toString(16);
		} catch (NoSuchAlgorithmException ex) {
		}
		scratchByteBuffer.clear();
		scratchIntBuffer.clear();

		try {
			for (int i = 0, n = hashes.size(); i < n; i++) {
				String other = hashes.get(i);
				if (other.equals(hash)) {
					Glyph dupe = pageGlyphs.get(i);
					glyph.setTexture(dupe.texture, dupe.u, dupe.v, dupe.u2, dupe.v2);
					return false;
				}
			}
		} finally {
			hashes.add(hash);
			pageGlyphs.add(glyph);
		}

		Gdx.gl.glTexSubImage2D(texture.glTarget, 0, pageX, pageY, width, height, format, GL11.GL_UNSIGNED_BYTE, glyphPixels);

		float u = pageX / (float)texture.getWidth();
		float v = pageY / (float)texture.getHeight();
		float u2 = (pageX + width) / (float)texture.getWidth();
		float v2 = (pageY + height) / (float)texture.getHeight();
		glyph.setTexture(texture, u, v, u2, v2);

		return true;
	}

	
	public List<Glyph> getGlyphs () {
		return pageGlyphs;
	}

	
	public Texture getTexture () {
		return texture;
	}

	static public final int MAX_GLYPH_SIZE = 256;

	static private ByteBuffer scratchByteBuffer = ByteBuffer.allocateDirect(MAX_GLYPH_SIZE * MAX_GLYPH_SIZE * 4);

	static {
		scratchByteBuffer.order(ByteOrder.LITTLE_ENDIAN);
	}

	static private IntBuffer scratchIntBuffer = scratchByteBuffer.asIntBuffer();

	static private BufferedImage scratchImage = new BufferedImage(MAX_GLYPH_SIZE, MAX_GLYPH_SIZE, BufferedImage.TYPE_INT_ARGB);
	static Graphics2D scratchGraphics = (Graphics2D)scratchImage.getGraphics();

	static {
		scratchGraphics.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		scratchGraphics.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
	}

	static public FontRenderContext renderContext = scratchGraphics.getFontRenderContext();
}
