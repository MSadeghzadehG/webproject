

package com.badlogic.gdx.tools.texturepacker;

import java.io.File;

public class TexturePackerUpscaleTest {
	public static void main (String[] args) {
		TexturePacker.Settings settings = new TexturePacker.Settings();
		settings.scale = new float[] {4};
		settings.scaleResampling = new TexturePacker.Resampling[] {TexturePacker.Resampling.nearest};

		TexturePacker packer = new TexturePacker(settings);
		packer.addImage(new File("tests/gdx-tests-gwt/war/assets/data/bobrgb888-32x32.png"));

		File out = new File("tmp/packout");

				if (out.exists()) {
			for (File f : out.listFiles()) {
				f.delete();
			}
		} else {
			out.mkdirs();
		}

		packer.pack(out, "main");
	}
}
