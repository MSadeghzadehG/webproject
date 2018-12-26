

package com.badlogic.gdx.tools.distancefield;

import java.awt.Color;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;


public class DistanceFieldGenerator {
	
	private Color color = Color.white;
	private int downscale = 1;
	private float spread = 1;
	
	
	public Color getColor() {
		return color;
	}
	
	
	public void setColor(Color color) {
		this.color = color;
	}
	
	
	public int getDownscale() {
		return downscale;
	}
	
	
	public void setDownscale(int downscale) {
		if (downscale <= 0)
			throw new IllegalArgumentException("downscale must be positive");
		this.downscale = downscale;
	}
	
	
	public float getSpread() {
		return spread;
	}
	
	
	public void setSpread(float spread) {
		if (spread <= 0)
			throw new IllegalArgumentException("spread must be positive");
		this.spread = spread;
	}
	
	
	private static int squareDist(final int x1, final int y1, final int x2, final int y2)
	{
		final int dx = x1 - x2;
		final int dy = y1 - y2;
		return dx*dx + dy*dy;
	}
	
	
	public BufferedImage generateDistanceField(BufferedImage inImage)
	{
		final int inWidth = inImage.getWidth();
		final int inHeight = inImage.getHeight();
		final int outWidth = inWidth / downscale;
		final int outHeight = inHeight / downscale;
		final BufferedImage outImage = new BufferedImage(outWidth, outHeight, BufferedImage.TYPE_4BYTE_ABGR);
		
				final boolean[][] bitmap = new boolean[inHeight][inWidth];
		for (int y = 0; y < inHeight; ++y) {
			for (int x = 0; x < inWidth; ++x) {
				bitmap[y][x] = isInside(inImage.getRGB(x, y));
			}
		}
		
		for (int y = 0; y < outHeight; ++y)
		{
			for (int x = 0; x < outWidth; ++x)
			{
				int centerX = (x * downscale) + (downscale / 2);
				int centerY = (y * downscale) + (downscale / 2);
				float signedDistance = findSignedDistance(centerX, centerY, bitmap);
				outImage.setRGB(x, y, distanceToRGB(signedDistance));
			}
		}
		
		return outImage;
	}
	
	
	private boolean isInside(int rgb) {
		return (rgb & 0x808080) != 0 && (rgb & 0x80000000) != 0;
	}
	
	
	private int distanceToRGB(float signedDistance) {
		float alpha = 0.5f + 0.5f * (signedDistance / spread);
		alpha = Math.min(1, Math.max(0, alpha)); 		int alphaByte = (int) (alpha * 0xFF); 		return (alphaByte << 24) | (color.getRGB() & 0xFFFFFF);
	}
	
	
	private float findSignedDistance(final int centerX, final int centerY, boolean[][] bitmap)
	{
		final int width = bitmap[0].length;
		final int height = bitmap.length;
		final boolean base = bitmap[centerY][centerX];
		
		final int delta = (int) Math.ceil(spread);
		final int startX = Math.max(0, centerX - delta);
		final int endX  = Math.min(width - 1, centerX + delta);
		final int startY = Math.max(0, centerY - delta);
		final int endY = Math.min(height - 1, centerY + delta);

		int closestSquareDist = delta * delta;
		
		for (int y = startY; y <= endY; ++y)
		{
			for (int x = startX; x <= endX; ++x)
			{
				if (base != bitmap[y][x])
				{
					final int squareDist = squareDist(centerX, centerY, x, y);
					if (squareDist < closestSquareDist)
					{
						closestSquareDist = squareDist;
					}
				}
			}
		}
		
		float closestDist = (float) Math.sqrt(closestSquareDist);
		return (base ? 1 : -1) * Math.min(closestDist, spread);
	}
	
	
	private static void usage() {
		System.out.println(
			"Generates a distance field image from a black and white input image.\n" +
		   "The distance field image contains a solid color and stores the distance\n" +
			"in the alpha channel.\n" +
		   "\n" +
		   "The output file format is inferred from the file name.\n" +
		   "\n" +
			"Command line arguments: INFILE OUTFILE [OPTION...]\n" +
		   "\n" +
			"Possible options:\n" +
			"  --color rrggbb    color of output image (default: ffffff)\n" +
			"  --downscale n     downscale by factor of n (default: 1)\n" +
			"  --spread n        edge scan distance (default: 1)\n");
	}
	
	
	private static class CommandLineArgumentException extends IllegalArgumentException {
		public CommandLineArgumentException(String message) {
			super(message);
		}
	}
	
	
	public static void main(String[] args) {
		try {
			run(args);
		} catch (CommandLineArgumentException e) {
			System.err.println("Error: " + e.getMessage() + "\n");
			usage();
			System.exit(1);
		}
	}
	
	
	private static void run(String[] args) {
		DistanceFieldGenerator generator = new DistanceFieldGenerator();
		String inputFile = null;
		String outputFile = null;
		
		int i = 0;
		try {
			for (; i < args.length; ++i) {
				String arg = args[i];
				if (arg.startsWith("-")) {
					if ("--help".equals(arg)) {
						usage();
						System.exit(0);
					} else if ("--color".equals(arg)) {
						++i;
						generator.setColor(new Color(Integer.parseInt(args[i], 16)));
					} else if ("--downscale".equals(arg)) {
						++i;
						generator.setDownscale(Integer.parseInt(args[i]));
					} else if ("--spread".equals(arg)) {
						++i;
						generator.setSpread(Float.parseFloat(args[i]));
					} else {
						throw new CommandLineArgumentException("unknown option " + arg);
					}
				} else {
					if (inputFile == null) {
						inputFile = arg;
					} else if (outputFile == null) {
						outputFile = arg;
					} else {
						throw new CommandLineArgumentException("exactly two file names are expected");
					}
				}
			}
		} catch (IndexOutOfBoundsException e) {
			throw new CommandLineArgumentException("option " + args[args.length - 1] + " requires an argument");
		} catch (NumberFormatException e) {
			throw new CommandLineArgumentException(args[i] + " is not a number");
		}
		if (inputFile == null) {
			throw new CommandLineArgumentException("no input file specified");
		}
		if (outputFile == null) {
			throw new CommandLineArgumentException("no output file specified");
		}
		
		String outputFormat = outputFile.substring(outputFile.lastIndexOf('.') + 1);
		boolean exists;
		if (!ImageIO.getImageWritersByFormatName(outputFormat).hasNext()) {
			throw new RuntimeException("No image writers found that can handle the format '" + outputFormat + "'");
		}
		
		BufferedImage input = null;
		try {
			input = ImageIO.read(new File(inputFile));
		} catch (IOException e) {
			System.err.println("Failed to load image: " + e.getMessage());
		}
		
		BufferedImage output = generator.generateDistanceField(input);
		
		try {
			ImageIO.write(output, outputFormat, new File(outputFile));
		} catch (IOException e) {
			System.err.println("Failed to write output image: " + e.getMessage());
		}
	}
}