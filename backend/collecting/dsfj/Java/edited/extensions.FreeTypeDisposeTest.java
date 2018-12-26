

package com.badlogic.gdx.tests.extensions;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.g2d.BitmapFont;
import com.badlogic.gdx.graphics.g2d.freetype.FreeTypeFontGenerator;
import com.badlogic.gdx.graphics.g2d.freetype.FreeTypeFontGenerator.FreeTypeBitmapFontData;
import com.badlogic.gdx.graphics.g2d.freetype.FreeTypeFontGenerator.FreeTypeFontParameter;
import com.badlogic.gdx.tests.utils.GdxTest;

public class FreeTypeDisposeTest extends GdxTest {
	BitmapFont font;

	@Override
	public void create () {
		super.create();
	}

	public void render () {
		if (Gdx.input.justTouched()) {
			for (int i = 0; i < 10; i++) {
				if (font != null) {
					font.dispose();
				}
				FileHandle fontFile = Gdx.files.internal("data/arial.ttf");
				FreeTypeFontGenerator generator = new FreeTypeFontGenerator(fontFile);

				FreeTypeFontParameter parameter = new FreeTypeFontParameter();
				parameter.size = 15;

				font = generator.generateFont(parameter);
				generator.dispose();
			}
			for (int i = 0; i < 10; i++)
				System.gc();
			Gdx.app.log("FreeTypeDisposeTest", "generated 10 fonts");
			Gdx.app.log("FreeTypeDisposeTest", Gdx.app.getJavaHeap() + ", " + Gdx.app.getNativeHeap());
		}
	}
}
