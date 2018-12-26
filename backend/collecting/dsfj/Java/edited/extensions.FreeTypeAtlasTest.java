

package com.badlogic.gdx.tests.extensions;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.OrthographicCamera;
import com.badlogic.gdx.graphics.Pixmap.Format;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.BitmapFont;
import com.badlogic.gdx.graphics.g2d.PixmapPacker;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;
import com.badlogic.gdx.graphics.g2d.TextureAtlas;
import com.badlogic.gdx.graphics.g2d.TextureRegion;
import com.badlogic.gdx.graphics.g2d.freetype.FreeTypeFontGenerator;
import com.badlogic.gdx.tests.utils.GdxTest;
import com.badlogic.gdx.utils.Array;

import java.util.EnumMap;


public class FreeTypeAtlasTest extends GdxTest {

		static enum FontSize {
		Tiny(10), Small(12), Medium(16), Large(20), Huge(24), ReallyHuge(28), JustTooBig(64);

		public final int size;

		FontSize(int size) {
			this.size = size;
		}
	}

		static enum FontStyle {
		Regular("data/arial.ttf"), Italic("data/arial-italic.ttf");

		public final String path;

		FontStyle(String path) {
			this.path = path;
		}
	}

	OrthographicCamera camera;
	SpriteBatch batch;
	String text;
	PixmapPacker packer;
	FontMap<BitmapFont> fontMap;

	public static final int FONT_ATLAS_WIDTH = 1024;
	public static final int FONT_ATLAS_HEIGHT = 512;

		private static final boolean INTEGER = false;

				public static final String CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" + "abcdefghijklmnopqrstuvwxyz\n1234567890"
			+ "\"!`?'.,;:()[]{}<>|/@\\^$-%+=#_&~*";

	@Override
	public void create() {
		camera = new OrthographicCamera();
		batch = new SpriteBatch();

		long start = System.currentTimeMillis();
		int glyphCount = createFonts();
		long time = System.currentTimeMillis() - start;
		text = glyphCount + " glyphs packed in " + packer.getPages().size + " page(s) in " + time + " ms";

	}

	@Override
	public void render() {
		Gdx.gl.glClearColor(0.2f, 0.2f, 0.2f, 1);
		Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT);

		camera.setToOrtho(false, Gdx.graphics.getWidth(), Gdx.graphics.getHeight());
		batch.setProjectionMatrix(camera.combined);
		batch.begin();

		float x = 10;
		float y = Gdx.graphics.getHeight() - 10;

		int renderCalls = 0;

						for (FontStyle style : FontStyle.values()) {
			for (FontSize size : FontSize.values()) {
				BitmapFont fnt = getFont(style, size);

				fnt.draw(batch, style.name() + " " + size.size + "pt: The quick brown fox jumps over the lazy dog", x, y);
				y -= fnt.getLineHeight() + 10;
			}
			y -= 20;
		}

		BitmapFont font = getFont(FontStyle.Regular, FontSize.Medium);
		font.draw(batch, text, 10, font.getCapHeight() + 10);

				batch.setColor(1f, 1f, 1f, 0.15f);
		batch.draw(packer.getPages().first().getTexture(), 0, 0);
		batch.setColor(1f, 1f, 1f, 1f);
		batch.end();
	}

	@Override
	public void dispose() {
		super.dispose();
		packer.dispose();
		batch.dispose();
	}

		public BitmapFont getFont(FontStyle style, FontSize size) {
		return fontMap.get(style).get(size);
	}

	protected int createFonts() {
																		
				packer = new PixmapPacker(FONT_ATLAS_WIDTH, FONT_ATLAS_HEIGHT, Format.RGBA8888, 2, false);

		fontMap = new FontMap<BitmapFont>();
		int fontCount = 0;

				for (FontStyle style : FontStyle.values()) {
						FreeTypeFontGenerator gen = new FreeTypeFontGenerator(Gdx.files.internal(style.path));

						for (FontSize size : FontSize.values()) {
								FreeTypeFontGenerator.FreeTypeFontParameter fontParameter = new FreeTypeFontGenerator.FreeTypeFontParameter();
				fontParameter.size = size.size;
				fontParameter.packer = packer;
				fontParameter.characters = CHARACTERS;
				BitmapFont bmFont = gen.generateFont(fontParameter);

				fontMap.get(style).put(size, bmFont);
				fontCount++;
			}

						gen.dispose();
		}

				return fontCount * CHARACTERS.length();
	}

		class FontMap<T> extends EnumMap<FontStyle, EnumMap<FontSize, T>> {

		public FontMap() {
			super(FontStyle.class);

						for (FontStyle style : FontStyle.values()) {
				put(style, new EnumMap<FontSize, T>(FontSize.class));
			}
		}
	}
}
