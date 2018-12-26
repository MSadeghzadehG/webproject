

package com.badlogic.gdx.graphics.g2d;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.StringTokenizer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.Color;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.Texture.TextureFilter;
import com.badlogic.gdx.graphics.g2d.GlyphLayout.GlyphRun;
import com.badlogic.gdx.graphics.g2d.TextureAtlas.AtlasRegion;
import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.FloatArray;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.StreamUtils;


public class BitmapFont implements Disposable {
	static private final int LOG2_PAGE_SIZE = 9;
	static private final int PAGE_SIZE = 1 << LOG2_PAGE_SIZE;
	static private final int PAGES = 0x10000 / PAGE_SIZE;

	final BitmapFontData data;
	Array<TextureRegion> regions;
	private final BitmapFontCache cache;
	private boolean flipped;
	boolean integer;
	private boolean ownsTexture;

	
	public BitmapFont () {
		this(Gdx.files.classpath("com/badlogic/gdx/utils/arial-15.fnt"), Gdx.files.classpath("com/badlogic/gdx/utils/arial-15.png"),
			false, true);
	}

	
	public BitmapFont (boolean flip) {
		this(Gdx.files.classpath("com/badlogic/gdx/utils/arial-15.fnt"), Gdx.files.classpath("com/badlogic/gdx/utils/arial-15.png"),
			flip, true);
	}

	
	public BitmapFont (FileHandle fontFile, TextureRegion region) {
		this(fontFile, region, false);
	}

	
	public BitmapFont (FileHandle fontFile, TextureRegion region, boolean flip) {
		this(new BitmapFontData(fontFile, flip), region, true);
	}

	
	public BitmapFont (FileHandle fontFile) {
		this(fontFile, false);
	}

	
	public BitmapFont (FileHandle fontFile, boolean flip) {
		this(new BitmapFontData(fontFile, flip), (TextureRegion)null, true);
	}

	
	public BitmapFont (FileHandle fontFile, FileHandle imageFile, boolean flip) {
		this(fontFile, imageFile, flip, true);
	}

	
	public BitmapFont (FileHandle fontFile, FileHandle imageFile, boolean flip, boolean integer) {
		this(new BitmapFontData(fontFile, flip), new TextureRegion(new Texture(imageFile, false)), integer);
		ownsTexture = true;
	}

	
	public BitmapFont (BitmapFontData data, TextureRegion region, boolean integer) {
		this(data, region != null ? Array.with(region) : null, integer);
	}

	
	public BitmapFont (BitmapFontData data, Array<TextureRegion> pageRegions, boolean integer) {
		this.flipped = data.flipped;
		this.data = data;
		this.integer = integer;

		if (pageRegions == null || pageRegions.size == 0) {
						int n = data.imagePaths.length;
			regions = new Array(n);
			for (int i = 0; i < n; i++) {
				FileHandle file;
				if (data.fontFile == null)
					file = Gdx.files.internal(data.imagePaths[i]);
				else
					file = Gdx.files.getFileHandle(data.imagePaths[i], data.fontFile.type());
				regions.add(new TextureRegion(new Texture(file, false)));
			}
			ownsTexture = true;
		} else {
			regions = pageRegions;
			ownsTexture = false;
		}

		cache = newFontCache();

		load(data);
	}

	protected void load (BitmapFontData data) {
		for (Glyph[] page : data.glyphs) {
			if (page == null) continue;
			for (Glyph glyph : page)
				if (glyph != null) data.setGlyphRegion(glyph, regions.get(glyph.page));
		}
		if (data.missingGlyph != null) data.setGlyphRegion(data.missingGlyph, regions.get(data.missingGlyph.page));
	}

	
	public GlyphLayout draw (Batch batch, CharSequence str, float x, float y) {
		cache.clear();
		GlyphLayout layout = cache.addText(str, x, y);
		cache.draw(batch);
		return layout;
	}

	
	public GlyphLayout draw (Batch batch, CharSequence str, float x, float y, float targetWidth, int halign, boolean wrap) {
		cache.clear();
		GlyphLayout layout = cache.addText(str, x, y, targetWidth, halign, wrap);
		cache.draw(batch);
		return layout;
	}

	
	public GlyphLayout draw (Batch batch, CharSequence str, float x, float y, int start, int end, float targetWidth, int halign,
		boolean wrap) {
		cache.clear();
		GlyphLayout layout = cache.addText(str, x, y, start, end, targetWidth, halign, wrap);
		cache.draw(batch);
		return layout;
	}

	
	public GlyphLayout draw (Batch batch, CharSequence str, float x, float y, int start, int end, float targetWidth, int halign,
		boolean wrap, String truncate) {
		cache.clear();
		GlyphLayout layout = cache.addText(str, x, y, start, end, targetWidth, halign, wrap, truncate);
		cache.draw(batch);
		return layout;
	}

	
	public void draw (Batch batch, GlyphLayout layout, float x, float y) {
		cache.clear();
		cache.addText(layout, x, y);
		cache.draw(batch);
	}

	
	public Color getColor () {
		return cache.getColor();
	}

	
	public void setColor (Color color) {
		cache.getColor().set(color);
	}

	
	public void setColor (float r, float g, float b, float a) {
		cache.getColor().set(r, g, b, a);
	}

	public float getScaleX () {
		return data.scaleX;
	}

	public float getScaleY () {
		return data.scaleY;
	}

	
	public TextureRegion getRegion () {
		return regions.first();
	}

	
	public Array<TextureRegion> getRegions () {
		return regions;
	}

	
	public TextureRegion getRegion (int index) {
		return regions.get(index);
	}

	
	public float getLineHeight () {
		return data.lineHeight;
	}

	
	public float getSpaceWidth () {
		return data.spaceWidth;
	}

	
	public float getXHeight () {
		return data.xHeight;
	}

	
	public float getCapHeight () {
		return data.capHeight;
	}

	
	public float getAscent () {
		return data.ascent;
	}

	
	public float getDescent () {
		return data.descent;
	}

	
	public boolean isFlipped () {
		return flipped;
	}

	
	public void dispose () {
		if (ownsTexture) {
			for (int i = 0; i < regions.size; i++)
				regions.get(i).getTexture().dispose();
		}
	}

	
	public void setFixedWidthGlyphs (CharSequence glyphs) {
		BitmapFontData data = this.data;
		int maxAdvance = 0;
		for (int index = 0, end = glyphs.length(); index < end; index++) {
			Glyph g = data.getGlyph(glyphs.charAt(index));
			if (g != null && g.xadvance > maxAdvance) maxAdvance = g.xadvance;
		}
		for (int index = 0, end = glyphs.length(); index < end; index++) {
			Glyph g = data.getGlyph(glyphs.charAt(index));
			if (g == null) continue;
			g.xoffset += Math.round((maxAdvance - g.xadvance) / 2);
			g.xadvance = maxAdvance;
			g.kerning = null;
			g.fixedWidth = true;
		}
	}

	
	public void setUseIntegerPositions (boolean integer) {
		this.integer = integer;
		cache.setUseIntegerPositions(integer);
	}

	
	public boolean usesIntegerPositions () {
		return integer;
	}

	
	public BitmapFontCache getCache () {
		return cache;
	}

	
	public BitmapFontData getData () {
		return data;
	}

	
	public boolean ownsTexture () {
		return ownsTexture;
	}

	
	public void setOwnsTexture (boolean ownsTexture) {
		this.ownsTexture = ownsTexture;
	}

	
	public BitmapFontCache newFontCache () {
		return new BitmapFontCache(this, integer);
	}

	public String toString () {
		if (data.fontFile != null) return data.fontFile.nameWithoutExtension();
		return super.toString();
	}

	
	public static class Glyph {
		public int id;
		public int srcX;
		public int srcY;
		public int width, height;
		public float u, v, u2, v2;
		public int xoffset, yoffset;
		public int xadvance;
		public byte[][] kerning;
		public boolean fixedWidth;

		
		public int page = 0;

		public int getKerning (char ch) {
			if (kerning != null) {
				byte[] page = kerning[ch >>> LOG2_PAGE_SIZE];
				if (page != null) return page[ch & PAGE_SIZE - 1];
			}
			return 0;
		}

		public void setKerning (int ch, int value) {
			if (kerning == null) kerning = new byte[PAGES][];
			byte[] page = kerning[ch >>> LOG2_PAGE_SIZE];
			if (page == null) kerning[ch >>> LOG2_PAGE_SIZE] = page = new byte[PAGE_SIZE];
			page[ch & PAGE_SIZE - 1] = (byte)value;
		}

		public String toString () {
			return Character.toString((char)id);
		}
	}

	static int indexOf (CharSequence text, char ch, int start) {
		final int n = text.length();
		for (; start < n; start++)
			if (text.charAt(start) == ch) return start;
		return n;
	}

	
	static public class BitmapFontData {
		
		public String[] imagePaths;
		public FileHandle fontFile;
		public boolean flipped;
		public float padTop, padRight, padBottom, padLeft;
		
		public float lineHeight;
		
		public float capHeight = 1;
		
		public float ascent;
		
		public float descent;
		
		public float down;
		
		public float blankLineScale = 1;
		public float scaleX = 1, scaleY = 1;
		public boolean markupEnabled;
		
		public float cursorX;

		public final Glyph[][] glyphs = new Glyph[PAGES][];
		
		public Glyph missingGlyph;

		
		public float spaceWidth;
		
		public float xHeight = 1;

		
		public char[] breakChars;
		public char[] xChars = {'x', 'e', 'a', 'o', 'n', 's', 'r', 'c', 'u', 'm', 'v', 'w', 'z'};
		public char[] capChars = {'M', 'N', 'B', 'D', 'C', 'E', 'F', 'K', 'A', 'G', 'H', 'I', 'J', 'L', 'O', 'P', 'Q', 'R', 'S',
			'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

		
		public BitmapFontData () {
		}

		public BitmapFontData (FileHandle fontFile, boolean flip) {
			this.fontFile = fontFile;
			this.flipped = flip;
			load(fontFile, flip);
		}

		public void load (FileHandle fontFile, boolean flip) {
			if (imagePaths != null) throw new IllegalStateException("Already loaded.");

			BufferedReader reader = new BufferedReader(new InputStreamReader(fontFile.read()), 512);
			try {
				String line = reader.readLine(); 				if (line == null) throw new GdxRuntimeException("File is empty.");

				line = line.substring(line.indexOf("padding=") + 8);
				String[] padding = line.substring(0, line.indexOf(' ')).split(",", 4);
				if (padding.length != 4) throw new GdxRuntimeException("Invalid padding.");
				padTop = Integer.parseInt(padding[0]);
				padRight = Integer.parseInt(padding[1]);
				padBottom = Integer.parseInt(padding[2]);
				padLeft = Integer.parseInt(padding[3]);
				float padY = padTop + padBottom;

				line = reader.readLine();
				if (line == null) throw new GdxRuntimeException("Missing common header.");
				String[] common = line.split(" ", 7); 
								if (common.length < 3) throw new GdxRuntimeException("Invalid common header.");

				if (!common[1].startsWith("lineHeight=")) throw new GdxRuntimeException("Missing: lineHeight");
				lineHeight = Integer.parseInt(common[1].substring(11));

				if (!common[2].startsWith("base=")) throw new GdxRuntimeException("Missing: base");
				float baseLine = Integer.parseInt(common[2].substring(5));

				int pageCount = 1;
				if (common.length >= 6 && common[5] != null && common[5].startsWith("pages=")) {
					try {
						pageCount = Math.max(1, Integer.parseInt(common[5].substring(6)));
					} catch (NumberFormatException ignored) { 					}
				}

				imagePaths = new String[pageCount];

								for (int p = 0; p < pageCount; p++) {
										line = reader.readLine();
					if (line == null) throw new GdxRuntimeException("Missing additional page definitions.");

										Matcher matcher = Pattern.compile(".*id=(\\d+)").matcher(line);
					if (matcher.find()) {
						String id = matcher.group(1);
						try {
							int pageID = Integer.parseInt(id);
							if (pageID != p) throw new GdxRuntimeException("Page IDs must be indices starting at 0: " + id);
						} catch (NumberFormatException ex) {
							throw new GdxRuntimeException("Invalid page id: " + id, ex);
						}
					}

					matcher = Pattern.compile(".*file=\"?([^\"]+)\"?").matcher(line);
					if (!matcher.find()) throw new GdxRuntimeException("Missing: file");
					String fileName = matcher.group(1);

					imagePaths[p] = fontFile.parent().child(fileName).path().replaceAll("\\\\", "/");
				}
				descent = 0;

				while (true) {
					line = reader.readLine();
					if (line == null) break; 					if (line.startsWith("kernings ")) break; 					if (!line.startsWith("char ")) continue;

					Glyph glyph = new Glyph();

					StringTokenizer tokens = new StringTokenizer(line, " =");
					tokens.nextToken();
					tokens.nextToken();
					int ch = Integer.parseInt(tokens.nextToken());
					if (ch <= 0)
						missingGlyph = glyph;
					else if (ch <= Character.MAX_VALUE)
						setGlyph(ch, glyph);
					else
						continue;
					glyph.id = ch;
					tokens.nextToken();
					glyph.srcX = Integer.parseInt(tokens.nextToken());
					tokens.nextToken();
					glyph.srcY = Integer.parseInt(tokens.nextToken());
					tokens.nextToken();
					glyph.width = Integer.parseInt(tokens.nextToken());
					tokens.nextToken();
					glyph.height = Integer.parseInt(tokens.nextToken());
					tokens.nextToken();
					glyph.xoffset = Integer.parseInt(tokens.nextToken());
					tokens.nextToken();
					if (flip)
						glyph.yoffset = Integer.parseInt(tokens.nextToken());
					else
						glyph.yoffset = -(glyph.height + Integer.parseInt(tokens.nextToken()));
					tokens.nextToken();
					glyph.xadvance = Integer.parseInt(tokens.nextToken());

										if (tokens.hasMoreTokens()) tokens.nextToken();
					if (tokens.hasMoreTokens()) {
						try {
							glyph.page = Integer.parseInt(tokens.nextToken());
						} catch (NumberFormatException ignored) {
						}
					}

					if (glyph.width > 0 && glyph.height > 0) descent = Math.min(baseLine + glyph.yoffset, descent);
				}
				descent += padBottom;

				while (true) {
					line = reader.readLine();
					if (line == null) break;
					if (!line.startsWith("kerning ")) break;

					StringTokenizer tokens = new StringTokenizer(line, " =");
					tokens.nextToken();
					tokens.nextToken();
					int first = Integer.parseInt(tokens.nextToken());
					tokens.nextToken();
					int second = Integer.parseInt(tokens.nextToken());
					if (first < 0 || first > Character.MAX_VALUE || second < 0 || second > Character.MAX_VALUE) continue;
					Glyph glyph = getGlyph((char)first);
					tokens.nextToken();
					int amount = Integer.parseInt(tokens.nextToken());
					if (glyph != null) { 						glyph.setKerning(second, amount);
					}
				}

				Glyph spaceGlyph = getGlyph(' ');
				if (spaceGlyph == null) {
					spaceGlyph = new Glyph();
					spaceGlyph.id = (int)' ';
					Glyph xadvanceGlyph = getGlyph('l');
					if (xadvanceGlyph == null) xadvanceGlyph = getFirstGlyph();
					spaceGlyph.xadvance = xadvanceGlyph.xadvance;
					setGlyph(' ', spaceGlyph);
				}
				if (spaceGlyph.width == 0) {
					spaceGlyph.width = (int)(padLeft + spaceGlyph.xadvance + padRight);
					spaceGlyph.xoffset = (int)-padLeft;
				}
				spaceWidth = spaceGlyph.width;

				Glyph xGlyph = null;
				for (char xChar : xChars) {
					xGlyph = getGlyph(xChar);
					if (xGlyph != null) break;
				}
				if (xGlyph == null) xGlyph = getFirstGlyph();
				xHeight = xGlyph.height - padY;

				Glyph capGlyph = null;
				for (char capChar : capChars) {
					capGlyph = getGlyph(capChar);
					if (capGlyph != null) break;
				}
				if (capGlyph == null) {
					for (Glyph[] page : this.glyphs) {
						if (page == null) continue;
						for (Glyph glyph : page) {
							if (glyph == null || glyph.height == 0 || glyph.width == 0) continue;
							capHeight = Math.max(capHeight, glyph.height);
						}
					}
				} else
					capHeight = capGlyph.height;
				capHeight -= padY;

				ascent = baseLine - capHeight;
				down = -lineHeight;
				if (flip) {
					ascent = -ascent;
					down = -down;
				}
			} catch (Exception ex) {
				throw new GdxRuntimeException("Error loading font file: " + fontFile, ex);
			} finally {
				StreamUtils.closeQuietly(reader);
			}
		}

		public void setGlyphRegion (Glyph glyph, TextureRegion region) {
			Texture texture = region.getTexture();
			float invTexWidth = 1.0f / texture.getWidth();
			float invTexHeight = 1.0f / texture.getHeight();

			float offsetX = 0, offsetY = 0;
			float u = region.u;
			float v = region.v;
			float regionWidth = region.getRegionWidth();
			float regionHeight = region.getRegionHeight();
			if (region instanceof AtlasRegion) {
								AtlasRegion atlasRegion = (AtlasRegion)region;
				offsetX = atlasRegion.offsetX;
				offsetY = atlasRegion.originalHeight - atlasRegion.packedHeight - atlasRegion.offsetY;
			}

			float x = glyph.srcX;
			float x2 = glyph.srcX + glyph.width;
			float y = glyph.srcY;
			float y2 = glyph.srcY + glyph.height;

						if (offsetX > 0) {
				x -= offsetX;
				if (x < 0) {
					glyph.width += x;
					glyph.xoffset -= x;
					x = 0;
				}
				x2 -= offsetX;
				if (x2 > regionWidth) {
					glyph.width -= x2 - regionWidth;
					x2 = regionWidth;
				}
			}
			if (offsetY > 0) {
				y -= offsetY;
				if (y < 0) {
					glyph.height += y;
					y = 0;
				}
				y2 -= offsetY;
				if (y2 > regionHeight) {
					float amount = y2 - regionHeight;
					glyph.height -= amount;
					glyph.yoffset += amount;
					y2 = regionHeight;
				}
			}

			glyph.u = u + x * invTexWidth;
			glyph.u2 = u + x2 * invTexWidth;
			if (flipped) {
				glyph.v = v + y * invTexHeight;
				glyph.v2 = v + y2 * invTexHeight;
			} else {
				glyph.v2 = v + y * invTexHeight;
				glyph.v = v + y2 * invTexHeight;
			}
		}

		
		public void setLineHeight (float height) {
			lineHeight = height * scaleY;
			down = flipped ? lineHeight : -lineHeight;
		}

		public void setGlyph (int ch, Glyph glyph) {
			Glyph[] page = glyphs[ch / PAGE_SIZE];
			if (page == null) glyphs[ch / PAGE_SIZE] = page = new Glyph[PAGE_SIZE];
			page[ch & PAGE_SIZE - 1] = glyph;
		}

		public Glyph getFirstGlyph () {
			for (Glyph[] page : this.glyphs) {
				if (page == null) continue;
				for (Glyph glyph : page) {
					if (glyph == null || glyph.height == 0 || glyph.width == 0) continue;
					return glyph;
				}
			}
			throw new GdxRuntimeException("No glyphs found.");
		}

		
		public boolean hasGlyph (char ch) {
			if (missingGlyph != null) return true;
			return getGlyph(ch) != null;
		}

		
		public Glyph getGlyph (char ch) {
			Glyph[] page = glyphs[ch / PAGE_SIZE];
			if (page != null) return page[ch & PAGE_SIZE - 1];
			return null;
		}

		
		public void getGlyphs (GlyphRun run, CharSequence str, int start, int end, boolean tightBounds) {
			boolean markupEnabled = this.markupEnabled;
			float scaleX = this.scaleX;
			Glyph missingGlyph = this.missingGlyph;
			Array<Glyph> glyphs = run.glyphs;
			FloatArray xAdvances = run.xAdvances;

						glyphs.ensureCapacity(end - start);
			xAdvances.ensureCapacity(end - start + 1);

			Glyph lastGlyph = null;
			while (start < end) {
				char ch = str.charAt(start++);
				Glyph glyph = getGlyph(ch);
				if (glyph == null) {
					if (missingGlyph == null) continue;
					glyph = missingGlyph;
				}

				glyphs.add(glyph);

				if (lastGlyph == null) 					xAdvances.add((!tightBounds || glyph.fixedWidth) ? 0 : -glyph.xoffset * scaleX - padLeft);
				else
					xAdvances.add((lastGlyph.xadvance + lastGlyph.getKerning(ch)) * scaleX);
				lastGlyph = glyph;

								if (markupEnabled && ch == '[' && start < end && str.charAt(start) == '[') start++;
			}
			if (lastGlyph != null) {
				float lastGlyphWidth = (!tightBounds || lastGlyph.fixedWidth) ? lastGlyph.xadvance
					: lastGlyph.xoffset + lastGlyph.width - padRight;
				xAdvances.add(lastGlyphWidth * scaleX);
			}
		}

		
		public int getWrapIndex (Array<Glyph> glyphs, int start) {
			int i = start - 1;
			for (; i >= 1; i--)
				if (!isWhitespace((char)glyphs.get(i).id)) break;
			for (; i >= 1; i--) {
				char ch = (char)glyphs.get(i).id;
				if (isWhitespace(ch) || isBreakChar(ch)) return i + 1;
			}
			return 0;
		}

		public boolean isBreakChar (char c) {
			if (breakChars == null) return false;
			for (char br : breakChars)
				if (c == br) return true;
			return false;
		}

		public boolean isWhitespace (char c) {
			switch (c) {
			case '\n':
			case '\r':
			case '\t':
			case ' ':
				return true;
			default:
				return false;
			}
		}

		
		public String getImagePath (int index) {
			return imagePaths[index];
		}

		public String[] getImagePaths () {
			return imagePaths;
		}

		public FileHandle getFontFile () {
			return fontFile;
		}

		
		public void setScale (float scaleX, float scaleY) {
			if (scaleX == 0) throw new IllegalArgumentException("scaleX cannot be 0.");
			if (scaleY == 0) throw new IllegalArgumentException("scaleY cannot be 0.");
			float x = scaleX / this.scaleX;
			float y = scaleY / this.scaleY;
			lineHeight *= y;
			spaceWidth *= x;
			xHeight *= y;
			capHeight *= y;
			ascent *= y;
			descent *= y;
			down *= y;
			padTop *= y;
			padLeft *= y;
			padBottom *= y;
			padRight *= y;
			this.scaleX = scaleX;
			this.scaleY = scaleY;
		}

		
		public void setScale (float scaleXY) {
			setScale(scaleXY, scaleXY);
		}

		
		public void scale (float amount) {
			setScale(scaleX + amount, scaleY + amount);
		}
	}
}
