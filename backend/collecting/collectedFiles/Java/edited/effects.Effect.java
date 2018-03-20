

package com.badlogic.gdx.tools.hiero.unicodefont.effects;

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;

import com.badlogic.gdx.tools.hiero.unicodefont.Glyph;
import com.badlogic.gdx.tools.hiero.unicodefont.UnicodeFont;


public interface Effect {
	
	public void draw (BufferedImage image, Graphics2D g, UnicodeFont unicodeFont, Glyph glyph);
}
