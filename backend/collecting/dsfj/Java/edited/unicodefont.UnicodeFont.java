

package com.badlogic.gdx.tools.hiero.unicodefont;

import java.awt.Font;
import java.awt.FontFormatException;
import java.awt.FontMetrics;
import java.awt.Rectangle;
import java.awt.font.GlyphVector;
import java.awt.font.TextAttribute;
import java.io.IOException;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.lwjgl.opengl.GL11;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.Color;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.BitmapFont;
import com.badlogic.gdx.graphics.g2d.BitmapFont.BitmapFontData;
import com.badlogic.gdx.graphics.g2d.BitmapFontCache;
import com.badlogic.gdx.graphics.g2d.GlyphLayout;
import com.badlogic.gdx.graphics.g2d.GlyphLayout.GlyphRun;
import com.badlogic.gdx.graphics.g2d.TextureRegion;
import com.badlogic.gdx.graphics.g2d.freetype.FreeTypeFontGenerator;
import com.badlogic.gdx.graphics.g2d.freetype.FreeTypeFontGenerator.FreeTypeFontParameter;
import com.badlogic.gdx.tools.hiero.HieroSettings;
import com.badlogic.gdx.tools.hiero.unicodefont.effects.Effect;
import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.GdxRuntimeException;



public class UnicodeFont {
	static private final int DISPLAY_LIST_CACHE_SIZE = 200;
	static private final int MAX_GLYPH_CODE = 0x10FFFF;
	static private final int PAGE_SIZE = 512;
	static private final int PAGES = MAX_GLYPH_CODE / PAGE_SIZE;

	private Font font;
	private FontMetrics metrics;
	private String ttfFileRef;
	private int ascent, descent, leading, spaceWidth;
	private final Glyph[][] glyphs = new Glyph[PAGES][];
	private final List<GlyphPage> glyphPages = new ArrayList();
	private final List<Glyph> queuedGlyphs = new ArrayList(256);
	private final List<Effect> effects = new ArrayList();
	private int paddingTop, paddingLeft, paddingBottom, paddingRight, paddingAdvanceX, paddingAdvanceY;
	private Glyph missingGlyph;
	private int glyphPageWidth = 512, glyphPageHeight = 512;
	RenderType renderType;

	BitmapFont bitmapFont;
	private FreeTypeFontGenerator generator;
	private BitmapFontCache cache;
	private GlyphLayout layout;
	private boolean mono;
	private float gamma;

	
	public UnicodeFont (String ttfFileRef, String hieroFileRef) {
		this(ttfFileRef, new HieroSettings(hieroFileRef));
	}

	
	public UnicodeFont (String ttfFileRef, HieroSettings settings) {
		this.ttfFileRef = ttfFileRef;
		Font font = createFont(ttfFileRef);
		initializeFont(font, settings.getFontSize(), settings.isBold(), settings.isItalic());
		loadSettings(settings);
	}

	
	public UnicodeFont (String ttfFileRef, int size, boolean bold, boolean italic) {
		this.ttfFileRef = ttfFileRef;
		initializeFont(createFont(ttfFileRef), size, bold, italic);
	}

	
	public UnicodeFont (Font font, String hieroFileRef) {
		this(font, new HieroSettings(hieroFileRef));
	}

	
	public UnicodeFont (Font font, HieroSettings settings) {
		initializeFont(font, settings.getFontSize(), settings.isBold(), settings.isItalic());
		loadSettings(settings);
	}

	
	public UnicodeFont (Font font) {
		initializeFont(font, font.getSize(), font.isBold(), font.isItalic());
	}

	
	public UnicodeFont (Font font, int size, boolean bold, boolean italic) {
		initializeFont(font, size, bold, italic);
	}

	private void initializeFont (Font baseFont, int size, boolean bold, boolean italic) {
		Map attributes = baseFont.getAttributes();
		attributes.put(TextAttribute.SIZE, new Float(size));
		attributes.put(TextAttribute.WEIGHT, bold ? TextAttribute.WEIGHT_BOLD : TextAttribute.WEIGHT_REGULAR);
		attributes.put(TextAttribute.POSTURE, italic ? TextAttribute.POSTURE_OBLIQUE : TextAttribute.POSTURE_REGULAR);
		try {
			attributes.put(TextAttribute.class.getDeclaredField("KERNING").get(null),
				TextAttribute.class.getDeclaredField("KERNING_ON").get(null));
		} catch (Throwable ignored) {
		}
		font = baseFont.deriveFont(attributes);

		metrics = GlyphPage.scratchGraphics.getFontMetrics(font);
		ascent = metrics.getAscent();
		descent = metrics.getDescent();
		leading = metrics.getLeading();

				char[] chars = " ".toCharArray();
		GlyphVector vector = font.layoutGlyphVector(GlyphPage.renderContext, chars, 0, chars.length, Font.LAYOUT_LEFT_TO_RIGHT);
		spaceWidth = vector.getGlyphLogicalBounds(0).getBounds().width;
	}

	private void loadSettings (HieroSettings settings) {
		paddingTop = settings.getPaddingTop();
		paddingLeft = settings.getPaddingLeft();
		paddingBottom = settings.getPaddingBottom();
		paddingRight = settings.getPaddingRight();
		paddingAdvanceX = settings.getPaddingAdvanceX();
		paddingAdvanceY = settings.getPaddingAdvanceY();
		glyphPageWidth = settings.getGlyphPageWidth();
		glyphPageHeight = settings.getGlyphPageHeight();
		effects.addAll(settings.getEffects());
	}

	
	public void addGlyphs (int startCodePoint, int endCodePoint) {
		for (int codePoint = startCodePoint; codePoint <= endCodePoint; codePoint++)
			addGlyphs(new String(Character.toChars(codePoint)));
	}

	
	public void addGlyphs (String text) {
		if (text == null) throw new IllegalArgumentException("text cannot be null.");

		char[] chars = text.toCharArray();
		GlyphVector vector = font.layoutGlyphVector(GlyphPage.renderContext, chars, 0, chars.length, Font.LAYOUT_LEFT_TO_RIGHT);
		for (int i = 0, n = vector.getNumGlyphs(); i < n; i++) {
			int codePoint = text.codePointAt(vector.getGlyphCharIndex(i));
			Rectangle bounds = getGlyphBounds(vector, i, codePoint);
			getGlyph(vector.getGlyphCode(i), codePoint, bounds, vector, i);
		}
	}

	
	public void addAsciiGlyphs () {
		addGlyphs(32, 255);
	}

	
	public void addNeheGlyphs () {
		addGlyphs(32, 32 + 96);
	}

	
	public boolean loadGlyphs () {
		return loadGlyphs(-1);
	}

	
	public boolean loadGlyphs (int maxGlyphsToLoad) {
		if (queuedGlyphs.isEmpty()) return false;

		if (effects.isEmpty())
			throw new IllegalStateException("The UnicodeFont must have at least one effect before any glyphs can be loaded.");

		for (Iterator iter = queuedGlyphs.iterator(); iter.hasNext();) {
			Glyph glyph = (Glyph)iter.next();
			int codePoint = glyph.getCodePoint();

						if (glyph.isMissing()) {
				if (missingGlyph != null) {
					if (glyph != missingGlyph) iter.remove();
					continue;
				}
				missingGlyph = glyph;
			}
		}

		Collections.sort(queuedGlyphs, heightComparator);

				for (Iterator iter = glyphPages.iterator(); iter.hasNext();) {
			GlyphPage glyphPage = (GlyphPage)iter.next();
			maxGlyphsToLoad -= glyphPage.loadGlyphs(queuedGlyphs, maxGlyphsToLoad);
			if (maxGlyphsToLoad == 0 || queuedGlyphs.isEmpty()) return true;
		}

				while (!queuedGlyphs.isEmpty()) {
			GlyphPage glyphPage = new GlyphPage(this, glyphPageWidth, glyphPageHeight);
			glyphPages.add(glyphPage);
			maxGlyphsToLoad -= glyphPage.loadGlyphs(queuedGlyphs, maxGlyphsToLoad);
			if (maxGlyphsToLoad == 0) return true;
		}

		return true;
	}

	
	public void dispose () {
		for (Iterator iter = glyphPages.iterator(); iter.hasNext();) {
			GlyphPage page = (GlyphPage)iter.next();
			page.getTexture().dispose();
		}
		if (bitmapFont != null) {
			bitmapFont.dispose();
			generator.dispose();
		}
	}

	public void drawString (float x, float y, String text, Color color, int startIndex, int endIndex) {
		if (text == null) throw new IllegalArgumentException("text cannot be null.");
		if (text.length() == 0) return;
		if (color == null) throw new IllegalArgumentException("color cannot be null.");

		x -= paddingLeft;
		y -= paddingTop;

		GL11.glColor4f(color.r, color.g, color.b, color.a);
		GL11.glTranslatef(x, y, 0);

		if (renderType == RenderType.FreeType && bitmapFont != null)
			drawBitmap(text, startIndex, endIndex);
		else
			drawUnicode(text, startIndex, endIndex);

		GL11.glTranslatef(-x, -y, 0);
	}

	static private final int X = 0, Y = 1, U = 3, V = 4;
	static private final int X2 = 10, Y2 = 11, U2 = 13, V2 = 14;

	private void drawBitmap (String text, int startIndex, int endIndex) {
		BitmapFontData data = bitmapFont.getData();
		int padY = paddingTop + paddingBottom + paddingAdvanceY;
		data.setLineHeight(data.lineHeight + padY);
		layout.setText(bitmapFont, text);
		data.setLineHeight(data.lineHeight - padY);
		for (GlyphRun run : layout.runs)
			for (int i = 0, n = run.xAdvances.size; i < n; i++)
				run.xAdvances.incr(i, paddingAdvanceX + paddingLeft + paddingRight);
		cache.setText(layout, paddingLeft, paddingRight);

		Array<TextureRegion> regions = bitmapFont.getRegions();
		for (int i = 0, n = regions.size; i < n; i++) {
			regions.get(i).getTexture().bind();
			GL11.glBegin(GL11.GL_QUADS);
			float[] vertices = cache.getVertices(i);
			for (int ii = 0, nn = vertices.length; ii < nn; ii += 20) {
				GL11.glTexCoord2f(vertices[ii + U], vertices[ii + V]);
				GL11.glVertex3f(vertices[ii + X], vertices[ii + Y], 0);
				GL11.glTexCoord2f(vertices[ii + U], vertices[ii + V2]);
				GL11.glVertex3f(vertices[ii + X], vertices[ii + Y2], 0);
				GL11.glTexCoord2f(vertices[ii + U2], vertices[ii + V2]);
				GL11.glVertex3f(vertices[ii + X2], vertices[ii + Y2], 0);
				GL11.glTexCoord2f(vertices[ii + U2], vertices[ii + V]);
				GL11.glVertex3f(vertices[ii + X2], vertices[ii + Y], 0);
			}
			GL11.glEnd();
		}
	}

	private void drawUnicode (String text, int startIndex, int endIndex) {
		char[] chars = text.substring(0, endIndex).toCharArray();
		GlyphVector vector = font.layoutGlyphVector(GlyphPage.renderContext, chars, 0, chars.length, Font.LAYOUT_LEFT_TO_RIGHT);

		int maxWidth = 0, totalHeight = 0, lines = 0;
		int extraX = 0, extraY = ascent;
		boolean startNewLine = false;
		Texture lastBind = null;
		int offsetX = 0;
		for (int glyphIndex = 0, n = vector.getNumGlyphs(); glyphIndex < n; glyphIndex++) {
			int charIndex = vector.getGlyphCharIndex(glyphIndex);
			if (charIndex < startIndex) continue;
			if (charIndex > endIndex) break;

			int codePoint = text.codePointAt(charIndex);

			Rectangle bounds = getGlyphBounds(vector, glyphIndex, codePoint);
			bounds.x += offsetX;
			Glyph glyph = getGlyph(vector.getGlyphCode(glyphIndex), codePoint, bounds, vector, glyphIndex);

			if (startNewLine && codePoint != '\n') {
				extraX = -bounds.x;
				startNewLine = false;
			}

			if (glyph.getTexture() == null && missingGlyph != null && glyph.isMissing()) glyph = missingGlyph;
			if (glyph.getTexture() != null) {
								Texture texture = glyph.getTexture();
				if (lastBind != null && lastBind != texture) {
					GL11.glEnd();
					lastBind = null;
				}
				if (lastBind == null) {
					texture.bind();
					GL11.glBegin(GL11.GL_QUADS);
					lastBind = texture;
				}
				int glyphX = bounds.x + extraX;
				int glyphY = bounds.y + extraY;
				GL11.glTexCoord2f(glyph.getU(), glyph.getV());
				GL11.glVertex3f(glyphX, glyphY, 0);
				GL11.glTexCoord2f(glyph.getU(), glyph.getV2());
				GL11.glVertex3f(glyphX, glyphY + glyph.getHeight(), 0);
				GL11.glTexCoord2f(glyph.getU2(), glyph.getV2());
				GL11.glVertex3f(glyphX + glyph.getWidth(), glyphY + glyph.getHeight(), 0);
				GL11.glTexCoord2f(glyph.getU2(), glyph.getV());
				GL11.glVertex3f(glyphX + glyph.getWidth(), glyphY, 0);
			}

			if (glyphIndex > 0) extraX += paddingRight + paddingLeft + paddingAdvanceX;
			maxWidth = Math.max(maxWidth, bounds.x + extraX + bounds.width);
			totalHeight = Math.max(totalHeight, ascent + bounds.y + bounds.height);

			if (codePoint == '\n') {
				startNewLine = true; 				extraY += getLineHeight();
				lines++;
				totalHeight = 0;
			} else if (renderType == RenderType.Native) offsetX += bounds.width;
		}
		if (lastBind != null) GL11.glEnd();
	}

	public void drawString (float x, float y, String text) {
		drawString(x, y, text, Color.WHITE);
	}

	public void drawString (float x, float y, String text, Color col) {
		drawString(x, y, text, col, 0, text.length());
	}

	
	public Glyph getGlyph (int glyphCode, int codePoint, Rectangle bounds, GlyphVector vector, int index) {
		if (glyphCode < 0 || glyphCode >= MAX_GLYPH_CODE) {
						return new Glyph(codePoint, bounds, vector, index, this) {
				public boolean isMissing () {
					return true;
				}
			};
		}
		int pageIndex = glyphCode / PAGE_SIZE;
		int glyphIndex = glyphCode & (PAGE_SIZE - 1);
		Glyph glyph = null;
		Glyph[] page = glyphs[pageIndex];
		if (page != null) {
			glyph = page[glyphIndex];
			if (glyph != null) return glyph;
		} else
			page = glyphs[pageIndex] = new Glyph[PAGE_SIZE];
				glyph = page[glyphIndex] = new Glyph(codePoint, bounds, vector, index, this);
		queuedGlyphs.add(glyph);
		return glyph;
	}

	private Rectangle getGlyphBounds (GlyphVector vector, int index, int codePoint) {
		Rectangle bounds;
		bounds = vector.getGlyphPixelBounds(index, GlyphPage.renderContext, 0, 0);
		if (renderType == RenderType.Native) {
			if (bounds.width == 0 || bounds.height == 0)
				bounds = new Rectangle();
			else
				bounds = metrics.getStringBounds("" + (char)codePoint, GlyphPage.scratchGraphics).getBounds();
		}
		if (codePoint == ' ') bounds.width = spaceWidth;
		return bounds;
	}

	public int getSpaceWidth () {
		return spaceWidth;
	}

	public int getWidth (String text) {
		if (text == null) throw new IllegalArgumentException("text cannot be null.");
		if (text.length() == 0) return 0;

		char[] chars = text.toCharArray();
		GlyphVector vector = font.layoutGlyphVector(GlyphPage.renderContext, chars, 0, chars.length, Font.LAYOUT_LEFT_TO_RIGHT);

		int width = 0;
		int extraX = 0;
		boolean startNewLine = false;
		for (int glyphIndex = 0, n = vector.getNumGlyphs(); glyphIndex < n; glyphIndex++) {
			int charIndex = vector.getGlyphCharIndex(glyphIndex);
			int codePoint = text.codePointAt(charIndex);
			Rectangle bounds = getGlyphBounds(vector, glyphIndex, codePoint);

			if (startNewLine && codePoint != '\n') extraX = -bounds.x;

			if (glyphIndex > 0) extraX += paddingLeft + paddingRight + paddingAdvanceX;
			width = Math.max(width, bounds.x + extraX + bounds.width);

			if (codePoint == '\n') startNewLine = true;
		}

		return width;
	}

	public int getHeight (String text) {
		if (text == null) throw new IllegalArgumentException("text cannot be null.");
		if (text.length() == 0) return 0;

		char[] chars = text.toCharArray();
		GlyphVector vector = font.layoutGlyphVector(GlyphPage.renderContext, chars, 0, chars.length, Font.LAYOUT_LEFT_TO_RIGHT);

		int lines = 0, height = 0;
		for (int i = 0, n = vector.getNumGlyphs(); i < n; i++) {
			int charIndex = vector.getGlyphCharIndex(i);
			int codePoint = text.codePointAt(charIndex);
			if (codePoint == ' ') continue;
			Rectangle bounds = getGlyphBounds(vector, i, codePoint);

			height = Math.max(height, ascent + bounds.y + bounds.height);

			if (codePoint == '\n') {
				lines++;
				height = 0;
			}
		}
		return lines * getLineHeight() + height;
	}

	
	public int getYOffset (String text) {
		if (text == null) throw new IllegalArgumentException("text cannot be null.");

		if (renderType == RenderType.FreeType && bitmapFont != null) return (int)bitmapFont.getAscent();

		int index = text.indexOf('\n');
		if (index != -1) text = text.substring(0, index);
		char[] chars = text.toCharArray();
		GlyphVector vector = font.layoutGlyphVector(GlyphPage.renderContext, chars, 0, chars.length, Font.LAYOUT_LEFT_TO_RIGHT);
		int yOffset = ascent + vector.getPixelBounds(null, 0, 0).y;

		return yOffset;
	}

	
	public Font getFont () {
		return font;
	}

	
	public int getPaddingTop () {
		return paddingTop;
	}

	
	public void setPaddingTop (int paddingTop) {
		this.paddingTop = paddingTop;
	}

	
	public int getPaddingLeft () {
		return paddingLeft;
	}

	
	public void setPaddingLeft (int paddingLeft) {
		this.paddingLeft = paddingLeft;
	}

	
	public int getPaddingBottom () {
		return paddingBottom;
	}

	
	public void setPaddingBottom (int paddingBottom) {
		this.paddingBottom = paddingBottom;
	}

	
	public int getPaddingRight () {
		return paddingRight;
	}

	
	public void setPaddingRight (int paddingRight) {
		this.paddingRight = paddingRight;
	}

	
	public int getPaddingAdvanceX () {
		return paddingAdvanceX;
	}

	
	public void setPaddingAdvanceX (int paddingAdvanceX) {
		this.paddingAdvanceX = paddingAdvanceX;
	}

	
	public int getPaddingAdvanceY () {
		return paddingAdvanceY;
	}

	
	public void setPaddingAdvanceY (int paddingAdvanceY) {
		this.paddingAdvanceY = paddingAdvanceY;
	}

	
	public int getLineHeight () {
		return descent + ascent + leading + paddingTop + paddingBottom + paddingAdvanceY;
	}

	
	public int getAscent () {
		return ascent;
	}

	
	public int getDescent () {
		return descent;
	}

	
	public int getLeading () {
		return leading;
	}

	
	public int getGlyphPageWidth () {
		return glyphPageWidth;
	}

	
	public void setGlyphPageWidth (int glyphPageWidth) {
		this.glyphPageWidth = glyphPageWidth;
	}

	
	public int getGlyphPageHeight () {
		return glyphPageHeight;
	}

	
	public void setGlyphPageHeight (int glyphPageHeight) {
		this.glyphPageHeight = glyphPageHeight;
	}

	
	public List getGlyphPages () {
		return glyphPages;
	}

	
	public List getEffects () {
		return effects;
	}

	public boolean getMono () {
		return mono;
	}

	public void setMono (boolean mono) {
		this.mono = mono;
	}

	public float getGamma () {
		return gamma;
	}

	public void setGamma (float gamma) {
		this.gamma = gamma;
	}

	public RenderType getRenderType () {
		return renderType;
	}

	public void setRenderType (RenderType renderType) {
		this.renderType = renderType;

		if (renderType != RenderType.FreeType) {
			if (bitmapFont != null) {
				bitmapFont.dispose();
				generator.dispose();
			}
		} else {
			String fontFile = getFontFile();
			if (fontFile != null) {
				generator = new FreeTypeFontGenerator(Gdx.files.absolute(fontFile));
				FreeTypeFontParameter param = new FreeTypeFontParameter();
				param.size = font.getSize();
				param.incremental = true;
				param.flip = true;
				param.mono = mono;
				param.gamma = gamma;
				bitmapFont = generator.generateFont(param);
				if (bitmapFont.getData().missingGlyph == null)
					bitmapFont.getData().missingGlyph = bitmapFont.getData().getGlyph('\ufffd');
				cache = bitmapFont.newFontCache();
				layout = new GlyphLayout();
			}
		}
	}

	
	public String getFontFile () {
		if (ttfFileRef == null) {
						try {
				Object font2D;
				try {
										font2D = Class.forName("sun.font.FontUtilities").getDeclaredMethod("getFont2D", new Class[] {Font.class})
						.invoke(null, new Object[] {font});
				} catch (Throwable ignored) {
					font2D = Class.forName("sun.font.FontManager").getDeclaredMethod("getFont2D", new Class[] {Font.class})
						.invoke(null, new Object[] {font});
				}
				Field platNameField = Class.forName("sun.font.PhysicalFont").getDeclaredField("platName");
				platNameField.setAccessible(true);
				ttfFileRef = (String)platNameField.get(font2D);
			} catch (Throwable ignored) {
			}
			if (ttfFileRef == null) ttfFileRef = "";
		}
		if (ttfFileRef.length() == 0) return null;
		return ttfFileRef;
	}

	
	static private Font createFont (String ttfFileRef) {
		try {
			return Font.createFont(Font.TRUETYPE_FONT, Gdx.files.absolute(ttfFileRef).read());
		} catch (FontFormatException ex) {
			throw new GdxRuntimeException("Invalid font: " + ttfFileRef, ex);
		} catch (IOException ex) {
			throw new GdxRuntimeException("Error reading font: " + ttfFileRef, ex);
		}
	}

	
	static private final Comparator heightComparator = new Comparator() {
		public int compare (Object o1, Object o2) {
			return ((Glyph)o2).getHeight() - ((Glyph)o1).getHeight();
		}
	};

	static public enum RenderType {
		Java, Native, FreeType
	}
}
