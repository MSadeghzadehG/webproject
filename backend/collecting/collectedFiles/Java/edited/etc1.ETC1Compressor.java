

package com.badlogic.gdx.tools.etc1;

import java.io.File;
import java.util.ArrayList;

import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.Pixmap;
import com.badlogic.gdx.graphics.Pixmap.Format;
import com.badlogic.gdx.graphics.glutils.ETC1;
import com.badlogic.gdx.tools.FileProcessor;
import com.badlogic.gdx.utils.GdxNativesLoader;

public class ETC1Compressor {
	static class ETC1FileProcessor extends FileProcessor {
		ETC1FileProcessor () {
			addInputSuffix(".png");
			addInputSuffix(".jpg");
			addInputSuffix(".jpeg");
			addInputSuffix(".bmp");
			setOutputSuffix(".etc1");
		}

		@Override
		protected void processFile (Entry entry) throws Exception {
			System.out.println("Processing " + entry.inputFile);
			Pixmap pixmap = new Pixmap(new FileHandle(entry.inputFile));
			if (pixmap.getFormat() != Format.RGB888 && pixmap.getFormat() != Format.RGB565) {
				System.out.println("Converting from " + pixmap.getFormat() + " to RGB888!");
				Pixmap tmp = new Pixmap(pixmap.getWidth(), pixmap.getHeight(), Format.RGB888);
				tmp.drawPixmap(pixmap, 0, 0, 0, 0, pixmap.getWidth(), pixmap.getHeight());
				pixmap.dispose();
				pixmap = tmp;
			}
			ETC1.encodeImagePKM(pixmap).write(new FileHandle(entry.outputFile));
			pixmap.dispose();
		}

		@Override
		protected void processDir (Entry entryDir, ArrayList<Entry> value) throws Exception {
			if (!entryDir.outputDir.exists()) {
				if (!entryDir.outputDir.mkdirs())
					throw new Exception("Couldn't create output directory '" + entryDir.outputDir + "'");
			}
		}
	}

	public static void process (String inputDirectory, String outputDirectory, boolean recursive, boolean flatten)
		throws Exception {
		GdxNativesLoader.load();
		ETC1FileProcessor processor = new ETC1FileProcessor();
		processor.setRecursive(recursive);
		processor.setFlattenOutput(flatten);
		processor.process(new File(inputDirectory), new File(outputDirectory));
	}

	public static void main (String[] args) throws Exception {
		if (args.length != 2) {
			System.out.println("ETC1Compressor <input-dir> <output-dir>");
			System.exit(-1);
		}
		ETC1Compressor.process(args[0], args[1], true, false);
	}
}
