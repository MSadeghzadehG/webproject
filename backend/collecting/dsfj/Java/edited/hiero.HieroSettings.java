

package com.badlogic.gdx.tools.hiero;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.tools.hiero.unicodefont.UnicodeFont;
import com.badlogic.gdx.tools.hiero.unicodefont.UnicodeFont.RenderType;
import com.badlogic.gdx.tools.hiero.unicodefont.effects.ConfigurableEffect;
import com.badlogic.gdx.tools.hiero.unicodefont.effects.ConfigurableEffect.Value;
import com.badlogic.gdx.utils.GdxRuntimeException;


public class HieroSettings {
	private static final String RENDER_TYPE = "render_type";
	private String fontName = "Arial";
	private int fontSize = 12;
	private boolean bold, italic, mono;
	private float gamma;
	private int paddingTop, paddingLeft, paddingBottom, paddingRight, paddingAdvanceX, paddingAdvanceY;
	private int glyphPageWidth = 512, glyphPageHeight = 512;
	private String glyphText = "";
	private final List effects = new ArrayList();
	private boolean nativeRendering;
	private boolean font2Active = false;
	private String font2File = "";
	private int renderType = RenderType.FreeType.ordinal();

	public HieroSettings () {
	}

	
	public HieroSettings (String hieroFileRef) {
		try {
			BufferedReader reader = new BufferedReader(new InputStreamReader(Gdx.files.absolute(hieroFileRef).read(), "UTF-8"));
			while (true) {
				String line = reader.readLine();
				if (line == null) break;
				line = line.trim();
				if (line.length() == 0) continue;
				String[] pieces = line.split("=", 2);
				String name = pieces[0].trim();
				String value = pieces[1];
				if (name.equals("font.name")) {
					fontName = value;
				} else if (name.equals("font.size")) {
					fontSize = Integer.parseInt(value);
				} else if (name.equals("font.gamma")) {
					gamma = Float.parseFloat(value);
				} else if (name.equals("font.mono")) {
					mono = Boolean.parseBoolean(value);
				} else if (name.equals("font.size")) {
					fontSize = Integer.parseInt(value);
				} else if (name.equals("font.bold")) {
					bold = Boolean.parseBoolean(value);
				} else if (name.equals("font.italic")) {
					italic = Boolean.parseBoolean(value);
				} else if (name.equals("font2.file")) {
					font2File = value;
				} else if (name.equals("font2.use")) {
					font2Active = Boolean.parseBoolean(value);
				} else if (name.equals("pad.top")) {
					paddingTop = Integer.parseInt(value);
				} else if (name.equals("pad.right")) {
					paddingRight = Integer.parseInt(value);
				} else if (name.equals("pad.bottom")) {
					paddingBottom = Integer.parseInt(value);
				} else if (name.equals("pad.left")) {
					paddingLeft = Integer.parseInt(value);
				} else if (name.equals("pad.advance.x")) {
					paddingAdvanceX = Integer.parseInt(value);
				} else if (name.equals("pad.advance.y")) {
					paddingAdvanceY = Integer.parseInt(value);
				} else if (name.equals("glyph.page.width")) {
					glyphPageWidth = Integer.parseInt(value);
				} else if (name.equals("glyph.page.height")) {
					glyphPageHeight = Integer.parseInt(value);
				} else if (name.equals("glyph.native.rendering")) {
					nativeRendering = Boolean.parseBoolean(value);
				} else if (name.equals("glyph.text")) {
					glyphText = value;
				} else if (name.equals(RENDER_TYPE)) {
					renderType = Integer.parseInt(value);
				} else if (name.equals("effect.class")) {
					try {
						effects.add(Class.forName(value).newInstance());
					} catch (Throwable ex) {
						throw new GdxRuntimeException("Unable to create effect instance: " + value, ex);
					}
				} else if (name.startsWith("effect.")) {
										name = name.substring(7);
					ConfigurableEffect effect = (ConfigurableEffect)effects.get(effects.size() - 1);
					List values = effect.getValues();
					for (Iterator iter = values.iterator(); iter.hasNext();) {
						Value effectValue = (Value)iter.next();
						if (effectValue.getName().equals(name)) {
							effectValue.setString(value);
							break;
						}
					}
					effect.setValues(values);
				}
			}
			reader.close();
		} catch (Throwable ex) {
			throw new GdxRuntimeException("Unable to load Hiero font file: " + hieroFileRef, ex);
		}
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

	
	public String getFontName () {
		return fontName;
	}

	
	public void setFontName (String fontName) {
		this.fontName = fontName;
	}

	
	public int getFontSize () {
		return fontSize;
	}

	
	public void setFontSize (int fontSize) {
		this.fontSize = fontSize;
	}

	
	public boolean isBold () {
		return bold;
	}

	
	public void setBold (boolean bold) {
		this.bold = bold;
	}

	
	public boolean isItalic () {
		return italic;
	}

	
	public void setItalic (boolean italic) {
		this.italic = italic;
	}

	
	public List getEffects () {
		return effects;
	}

	public boolean getNativeRendering () {
		return nativeRendering;
	}

	public void setNativeRendering (boolean nativeRendering) {
		this.nativeRendering = nativeRendering;
	}

	public String getGlyphText () {
		return this.glyphText.replace("\\n", "\n");
	}

	public void setGlyphText (String text) {
		this.glyphText = text.replace("\n", "\\n");
	}

	public String getFont2File () {
		return font2File;
	}

	public void setFont2File (String filename) {
		this.font2File = filename;
	}

	public boolean isFont2Active () {
		return font2Active;
	}

	public void setFont2Active (boolean active) {
		this.font2Active = active;
	}

	public boolean isMono () {
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

	
	public void save (File file) throws IOException {
		PrintStream out = new PrintStream(file, "UTF-8");
		out.println("font.name=" + fontName);
		out.println("font.size=" + fontSize);
		out.println("font.bold=" + bold);
		out.println("font.italic=" + italic);
		out.println("font.gamma=" + gamma);
		out.println("font.mono=" + mono);
		out.println();
		out.println("font2.file=" + font2File);
		out.println("font2.use=" + font2Active);
		out.println();
		out.println("pad.top=" + paddingTop);
		out.println("pad.right=" + paddingRight);
		out.println("pad.bottom=" + paddingBottom);
		out.println("pad.left=" + paddingLeft);
		out.println("pad.advance.x=" + paddingAdvanceX);
		out.println("pad.advance.y=" + paddingAdvanceY);
		out.println();
		out.println("glyph.native.rendering=" + nativeRendering);
		out.println("glyph.page.width=" + glyphPageWidth);
		out.println("glyph.page.height=" + glyphPageHeight);
		out.println("glyph.text=" + glyphText);
		out.println();
		out.println(RENDER_TYPE + "=" + renderType);
		out.println();
		for (Iterator iter = effects.iterator(); iter.hasNext();) {
			ConfigurableEffect effect = (ConfigurableEffect)iter.next();
			out.println("effect.class=" + effect.getClass().getName());
			for (Iterator iter2 = effect.getValues().iterator(); iter2.hasNext();) {
				Value value = (Value)iter2.next();
				out.println("effect." + value.getName() + "=" + value.getString());
			}
			out.println();
		}
		out.close();
	}

	public void setRenderType (int renderType) {
		this.renderType = renderType;
	}

	public int getRenderType () {
		return renderType;
	}
}
