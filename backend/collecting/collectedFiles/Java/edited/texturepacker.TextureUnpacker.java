

package com.badlogic.gdx.tools.texturepacker;

import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.g2d.TextureAtlas.TextureAtlasData;
import com.badlogic.gdx.graphics.g2d.TextureAtlas.TextureAtlasData.Page;
import com.badlogic.gdx.graphics.g2d.TextureAtlas.TextureAtlasData.Region;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.AffineTransform;
import java.awt.image.AffineTransformOp;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;


public class TextureUnpacker {
	private static final String DEFAULT_OUTPUT_PATH = "output";
	private static final int NINEPATCH_PADDING = 1;
	private static final String OUTPUT_TYPE = "png";
	private static final String HELP = "Usage: atlasFile [imageDir] [outputDir]";
	private static final String ATLAS_FILE_EXTENSION = ".atlas";

	
	private int parseArguments (String[] args) {
		int numArgs = args.length;
				if (numArgs < 1) return 0;
				boolean extension = args[0].substring(args[0].length() - ATLAS_FILE_EXTENSION.length()).equals(ATLAS_FILE_EXTENSION);
				boolean directory = true;
		if (numArgs >= 2) directory &= checkDirectoryValidity(args[1]);
		if (numArgs == 3) directory &= checkDirectoryValidity(args[2]);
		return extension && directory ? numArgs : 0;
	}

	private boolean checkDirectoryValidity (String directory) {
		File checkFile = new File(directory);
		boolean path = true;
				try {
			checkFile.getCanonicalPath();
		} catch (Exception e) {
			path = false;
		}
		return path;
	}

	
	public void splitAtlas (TextureAtlasData atlas, String outputDir) {
				File outputDirFile = new File(outputDir);
		if (!outputDirFile.exists()) {
			outputDirFile.mkdirs();
			System.out.println(String.format("Creating directory: %s", outputDirFile.getPath()));
		}

		for (Page page : atlas.getPages()) {
						File file = page.textureFile.file();
			if (!file.exists()) throw new RuntimeException("Unable to find atlas image: " + file.getAbsolutePath());
			BufferedImage img = null;
			try {
				img = ImageIO.read(file);
			} catch (IOException e) {
				printExceptionAndExit(e);
			}
			for (Region region : atlas.getRegions()) {
				System.out.println(String.format("Processing image for %s: x[%s] y[%s] w[%s] h[%s], rotate[%s]", region.name,
					region.left, region.top, region.width, region.height, region.rotate));

								if (region.page == page) {
					BufferedImage splitImage = null;
					String extension = null;

										if (region.splits == null) {
						splitImage = extractImage(img, region, outputDirFile, 0);
						extension = OUTPUT_TYPE;
					} else {
						splitImage = extractNinePatch(img, region, outputDirFile);
						extension = String.format("9.%s", OUTPUT_TYPE);
					}

										File imgOutput = new File(outputDirFile,
						String.format("%s.%s", region.index == -1 ? region.name : region.name + "_" + region.index, extension));
					File imgDir = imgOutput.getParentFile();
					if (!imgDir.exists()) {
						System.out.println(String.format("Creating directory: %s", imgDir.getPath()));
						imgDir.mkdirs();
					}

										try {
						ImageIO.write(splitImage, OUTPUT_TYPE, imgOutput);
					} catch (Exception e) {
						printExceptionAndExit(e);
					}
				}
			}
		}
	}

	
	private BufferedImage extractImage (BufferedImage page, Region region, File outputDirFile, int padding) {
		BufferedImage splitImage = null;

				if (region.rotate) {
			BufferedImage srcImage = page.getSubimage(region.left, region.top, region.height, region.width);
			splitImage = new BufferedImage(region.width, region.height, page.getType());

			AffineTransform transform = new AffineTransform();
			transform.rotate(Math.toRadians(90.0));
			transform.translate(0, -region.width);
			AffineTransformOp op = new AffineTransformOp(transform, AffineTransformOp.TYPE_BILINEAR);
			op.filter(srcImage, splitImage);
		} else {
			splitImage = page.getSubimage(region.left, region.top, region.width, region.height);
		}

				if (padding > 0) {
			BufferedImage paddedImage = new BufferedImage(splitImage.getWidth() + padding * 2, splitImage.getHeight() + padding * 2,
				page.getType());
			Graphics2D g2 = paddedImage.createGraphics();
			g2.drawImage(splitImage, padding, padding, null);
			g2.dispose();
			return paddedImage;
		} else {
			return splitImage;
		}
	}

	
	private BufferedImage extractNinePatch (BufferedImage page, Region region, File outputDirFile) {
		BufferedImage splitImage = extractImage(page, region, outputDirFile, NINEPATCH_PADDING);
		Graphics2D g2 = splitImage.createGraphics();
		g2.setColor(Color.BLACK);

				int startX = region.splits[0] + NINEPATCH_PADDING;
		int endX = region.width - region.splits[1] + NINEPATCH_PADDING - 1;
		int startY = region.splits[2] + NINEPATCH_PADDING;
		int endY = region.height - region.splits[3] + NINEPATCH_PADDING - 1;
		if (endX >= startX) g2.drawLine(startX, 0, endX, 0);
		if (endY >= startY) g2.drawLine(0, startY, 0, endY);
		if (region.pads != null) {
			int padStartX = region.pads[0] + NINEPATCH_PADDING;
			int padEndX = region.width - region.pads[1] + NINEPATCH_PADDING - 1;
			int padStartY = region.pads[2] + NINEPATCH_PADDING;
			int padEndY = region.height - region.pads[3] + NINEPATCH_PADDING - 1;
			g2.drawLine(padStartX, splitImage.getHeight() - 1, padEndX, splitImage.getHeight() - 1);
			g2.drawLine(splitImage.getWidth() - 1, padStartY, splitImage.getWidth() - 1, padEndY);
		}
		g2.dispose();

		return splitImage;
	}

	private void printExceptionAndExit (Exception e) {
		e.printStackTrace();
		System.exit(1);
	}

	public static void main (String[] args) {
		TextureUnpacker unpacker = new TextureUnpacker();

		String atlasFile = null, imageDir = null, outputDir = null;

				switch (unpacker.parseArguments(args)) {
		case 0:
			System.out.println(HELP);
			return;
		case 3:
			outputDir = args[2];
		case 2:
			imageDir = args[1];
		case 1:
			atlasFile = args[0];
		}

		File atlasFileHandle = new File(atlasFile).getAbsoluteFile();
		if (!atlasFileHandle.exists()) throw new RuntimeException("Atlas file not found: " + atlasFileHandle.getAbsolutePath());
		String atlasParentPath = atlasFileHandle.getParentFile().getAbsolutePath();

				if (imageDir == null) imageDir = atlasParentPath;
		if (outputDir == null) outputDir = (new File(atlasParentPath, DEFAULT_OUTPUT_PATH)).getAbsolutePath();

				TextureAtlasData atlas = new TextureAtlasData(new FileHandle(atlasFile), new FileHandle(imageDir), false);
		unpacker.splitAtlas(atlas, outputDir);
	}
}
