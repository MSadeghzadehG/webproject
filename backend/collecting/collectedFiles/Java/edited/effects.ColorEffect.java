

package com.badlogic.gdx.tools.hiero.unicodefont.effects;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.badlogic.gdx.tools.hiero.unicodefont.Glyph;
import com.badlogic.gdx.tools.hiero.unicodefont.UnicodeFont;


public class ColorEffect implements ConfigurableEffect {
	private Color color = Color.white;

	public ColorEffect () {
	}

	public ColorEffect (Color color) {
		this.color = color;
	}

	public void draw (BufferedImage image, Graphics2D g, UnicodeFont unicodeFont, Glyph glyph) {
		g.setColor(color);
		try {
			g.fill(glyph.getShape()); 		} catch (Throwable ignored) {
		}
	}

	public Color getColor () {
		return color;
	}

	public void setColor (Color color) {
		if (color == null) throw new IllegalArgumentException("color cannot be null.");
		this.color = color;
	}

	public String toString () {
		return "Color";
	}

	public List getValues () {
		List values = new ArrayList();
		values.add(EffectUtil.colorValue("Color", color));
		return values;
	}

	public void setValues (List values) {
		for (Iterator iter = values.iterator(); iter.hasNext();) {
			Value value = (Value)iter.next();
			if (value.getName().equals("Color")) {
				setColor((Color)value.getObject());
			}
		}
	}
}
