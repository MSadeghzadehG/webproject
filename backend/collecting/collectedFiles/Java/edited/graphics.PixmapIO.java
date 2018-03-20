

package com.badlogic.gdx.graphics;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.zip.CRC32;
import java.util.zip.CheckedOutputStream;
import java.util.zip.Deflater;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.InflaterInputStream;

import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.Pixmap.Format;
import com.badlogic.gdx.utils.ByteArray;
import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.StreamUtils;


public class PixmapIO {
	
	static public void writeCIM (FileHandle file, Pixmap pixmap) {
		CIM.write(file, pixmap);
	}

	
	static public Pixmap readCIM (FileHandle file) {
		return CIM.read(file);
	}

	
	static public void writePNG (FileHandle file, Pixmap pixmap) {
		try {
			PNG writer = new PNG((int)(pixmap.getWidth() * pixmap.getHeight() * 1.5f)); 			try {
				writer.setFlipY(false);
				writer.write(file, pixmap);
			} finally {
				writer.dispose();
			}
		} catch (IOException ex) {
			throw new GdxRuntimeException("Error writing PNG: " + file, ex);
		}
	}

	
	static private class CIM {
		static private final int BUFFER_SIZE = 32000;
		static private final byte[] writeBuffer = new byte[BUFFER_SIZE];
		static private final byte[] readBuffer = new byte[BUFFER_SIZE];

		static public void write (FileHandle file, Pixmap pixmap) {
			DataOutputStream out = null;

			try {
								DeflaterOutputStream deflaterOutputStream = new DeflaterOutputStream(file.write(false));
				out = new DataOutputStream(deflaterOutputStream);
				out.writeInt(pixmap.getWidth());
				out.writeInt(pixmap.getHeight());
				out.writeInt(Format.toGdx2DPixmapFormat(pixmap.getFormat()));

				ByteBuffer pixelBuf = pixmap.getPixels();
				pixelBuf.position(0);
				pixelBuf.limit(pixelBuf.capacity());

				int remainingBytes = pixelBuf.capacity() % BUFFER_SIZE;
				int iterations = pixelBuf.capacity() / BUFFER_SIZE;

				synchronized (writeBuffer) {
					for (int i = 0; i < iterations; i++) {
						pixelBuf.get(writeBuffer);
						out.write(writeBuffer);
					}

					pixelBuf.get(writeBuffer, 0, remainingBytes);
					out.write(writeBuffer, 0, remainingBytes);
				}

				pixelBuf.position(0);
				pixelBuf.limit(pixelBuf.capacity());
											} catch (Exception e) {
				throw new GdxRuntimeException("Couldn't write Pixmap to file '" + file + "'", e);
			} finally {
				StreamUtils.closeQuietly(out);
			}
		}

		static public Pixmap read (FileHandle file) {
			DataInputStream in = null;

			try {
								in = new DataInputStream(new InflaterInputStream(new BufferedInputStream(file.read())));
				int width = in.readInt();
				int height = in.readInt();
				Format format = Format.fromGdx2DPixmapFormat(in.readInt());
				Pixmap pixmap = new Pixmap(width, height, format);

				ByteBuffer pixelBuf = pixmap.getPixels();
				pixelBuf.position(0);
				pixelBuf.limit(pixelBuf.capacity());

				synchronized (readBuffer) {
					int readBytes = 0;
					while ((readBytes = in.read(readBuffer)) > 0) {
						pixelBuf.put(readBuffer, 0, readBytes);
					}
				}

				pixelBuf.position(0);
				pixelBuf.limit(pixelBuf.capacity());
								return pixmap;
			} catch (Exception e) {
				throw new GdxRuntimeException("Couldn't read Pixmap from file '" + file + "'", e);
			} finally {
				StreamUtils.closeQuietly(in);
			}
		}
	}

	
	static public class PNG implements Disposable {
		static private final byte[] SIGNATURE = {(byte)137, 80, 78, 71, 13, 10, 26, 10};
		static private final int IHDR = 0x49484452, IDAT = 0x49444154, IEND = 0x49454E44;
		static private final byte COLOR_ARGB = 6;
		static private final byte COMPRESSION_DEFLATE = 0;
		static private final byte FILTER_NONE = 0;
		static private final byte INTERLACE_NONE = 0;
		static private final byte PAETH = 4;

		private final ChunkBuffer buffer;
		private final Deflater deflater;
		private ByteArray lineOutBytes, curLineBytes, prevLineBytes;
		private boolean flipY = true;
		private int lastLineLen;

		public PNG () {
			this(128 * 128);
		}

		public PNG (int initialBufferSize) {
			buffer = new ChunkBuffer(initialBufferSize);
			deflater = new Deflater();
		}

		
		public void setFlipY (boolean flipY) {
			this.flipY = flipY;
		}

		
		public void setCompression (int level) {
			deflater.setLevel(level);
		}

		public void write (FileHandle file, Pixmap pixmap) throws IOException {
			OutputStream output = file.write(false);
			try {
				write(output, pixmap);
			} finally {
				StreamUtils.closeQuietly(output);
			}
		}

		
		public void write (OutputStream output, Pixmap pixmap) throws IOException {
			DeflaterOutputStream deflaterOutput = new DeflaterOutputStream(buffer, deflater);
			DataOutputStream dataOutput = new DataOutputStream(output);
			dataOutput.write(SIGNATURE);

			buffer.writeInt(IHDR);
			buffer.writeInt(pixmap.getWidth());
			buffer.writeInt(pixmap.getHeight());
			buffer.writeByte(8); 			buffer.writeByte(COLOR_ARGB);
			buffer.writeByte(COMPRESSION_DEFLATE);
			buffer.writeByte(FILTER_NONE);
			buffer.writeByte(INTERLACE_NONE);
			buffer.endChunk(dataOutput);

			buffer.writeInt(IDAT);
			deflater.reset();

			int lineLen = pixmap.getWidth() * 4;
			byte[] lineOut, curLine, prevLine;
			if (lineOutBytes == null) {
				lineOut = (lineOutBytes = new ByteArray(lineLen)).items;
				curLine = (curLineBytes = new ByteArray(lineLen)).items;
				prevLine = (prevLineBytes = new ByteArray(lineLen)).items;
			} else {
				lineOut = lineOutBytes.ensureCapacity(lineLen);
				curLine = curLineBytes.ensureCapacity(lineLen);
				prevLine = prevLineBytes.ensureCapacity(lineLen);
				for (int i = 0, n = lastLineLen; i < n; i++)
					prevLine[i] = 0;
			}
			lastLineLen = lineLen;

			ByteBuffer pixels = pixmap.getPixels();
			int oldPosition = pixels.position();
			boolean rgba8888 = pixmap.getFormat() == Format.RGBA8888;
			for (int y = 0, h = pixmap.getHeight(); y < h; y++) {
				int py = flipY ? (h - y - 1) : y;
				if (rgba8888) {
					pixels.position(py * lineLen);
					pixels.get(curLine, 0, lineLen);
				} else {
					for (int px = 0, x = 0; px < pixmap.getWidth(); px++) {
						int pixel = pixmap.getPixel(px, py);
						curLine[x++] = (byte)((pixel >> 24) & 0xff);
						curLine[x++] = (byte)((pixel >> 16) & 0xff);
						curLine[x++] = (byte)((pixel >> 8) & 0xff);
						curLine[x++] = (byte)(pixel & 0xff);
					}
				}

				lineOut[0] = (byte)(curLine[0] - prevLine[0]);
				lineOut[1] = (byte)(curLine[1] - prevLine[1]);
				lineOut[2] = (byte)(curLine[2] - prevLine[2]);
				lineOut[3] = (byte)(curLine[3] - prevLine[3]);

				for (int x = 4; x < lineLen; x++) {
					int a = curLine[x - 4] & 0xff;
					int b = prevLine[x] & 0xff;
					int c = prevLine[x - 4] & 0xff;
					int p = a + b - c;
					int pa = p - a;
					if (pa < 0) pa = -pa;
					int pb = p - b;
					if (pb < 0) pb = -pb;
					int pc = p - c;
					if (pc < 0) pc = -pc;
					if (pa <= pb && pa <= pc)
						c = a;
					else if (pb <= pc) 						c = b;
					lineOut[x] = (byte)(curLine[x] - c);
				}

				deflaterOutput.write(PAETH);
				deflaterOutput.write(lineOut, 0, lineLen);

				byte[] temp = curLine;
				curLine = prevLine;
				prevLine = temp;
			}
			pixels.position(oldPosition);
			deflaterOutput.finish();
			buffer.endChunk(dataOutput);

			buffer.writeInt(IEND);
			buffer.endChunk(dataOutput);

			output.flush();
		}

		
		public void dispose () {
			deflater.end();
		}

		static class ChunkBuffer extends DataOutputStream {
			final ByteArrayOutputStream buffer;
			final CRC32 crc;

			ChunkBuffer (int initialSize) {
				this(new ByteArrayOutputStream(initialSize), new CRC32());
			}

			private ChunkBuffer (ByteArrayOutputStream buffer, CRC32 crc) {
				super(new CheckedOutputStream(buffer, crc));
				this.buffer = buffer;
				this.crc = crc;
			}

			public void endChunk (DataOutputStream target) throws IOException {
				flush();
				target.writeInt(buffer.size() - 4);
				buffer.writeTo(target);
				target.writeInt((int)crc.getValue());
				buffer.reset();
				crc.reset();
			}
		}
	}
}
