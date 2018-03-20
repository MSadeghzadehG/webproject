

package com.badlogic.gdx.graphics.glutils;

import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.nio.ByteBuffer;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.Pixmap;
import com.badlogic.gdx.graphics.Pixmap.Format;
import com.badlogic.gdx.math.MathUtils;
import com.badlogic.gdx.utils.BufferUtils;
import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.StreamUtils;


public class ETC1 {
	
	public static int PKM_HEADER_SIZE = 16;
	public static int ETC1_RGB8_OES = 0x00008d64;

	
	public final static class ETC1Data implements Disposable {
		
		public final int width;
		
		public final int height;
		
		public final ByteBuffer compressedData;
		
		public final int dataOffset;

		public ETC1Data (int width, int height, ByteBuffer compressedData, int dataOffset) {
			this.width = width;
			this.height = height;
			this.compressedData = compressedData;
			this.dataOffset = dataOffset;
			checkNPOT();
		}

		public ETC1Data (FileHandle pkmFile) {
			byte[] buffer = new byte[1024 * 10];
			DataInputStream in = null;
			try {
				in = new DataInputStream(new BufferedInputStream(new GZIPInputStream(pkmFile.read())));
				int fileSize = in.readInt();
				compressedData = BufferUtils.newUnsafeByteBuffer(fileSize);
				int readBytes = 0;
				while ((readBytes = in.read(buffer)) != -1) {
					compressedData.put(buffer, 0, readBytes);
				}
				compressedData.position(0);
				compressedData.limit(compressedData.capacity());
			} catch (Exception e) {
				throw new GdxRuntimeException("Couldn't load pkm file '" + pkmFile + "'", e);
			} finally {
				StreamUtils.closeQuietly(in);
			}

			width = getWidthPKM(compressedData, 0);
			height = getHeightPKM(compressedData, 0);
			dataOffset = PKM_HEADER_SIZE;
			compressedData.position(dataOffset);
			checkNPOT();
		}

		private void checkNPOT () {
			if (!MathUtils.isPowerOfTwo(width) || !MathUtils.isPowerOfTwo(height)) {
				System.out.println("ETC1Data " + "warning: non-power-of-two ETC1 textures may crash the driver of PowerVR GPUs");
			}
		}

		
		public boolean hasPKMHeader () {
			return dataOffset == 16;
		}

		
		public void write (FileHandle file) {
			DataOutputStream write = null;
			byte[] buffer = new byte[10 * 1024];
			int writtenBytes = 0;
			compressedData.position(0);
			compressedData.limit(compressedData.capacity());
			try {
				write = new DataOutputStream(new GZIPOutputStream(file.write(false)));
				write.writeInt(compressedData.capacity());
				while (writtenBytes != compressedData.capacity()) {
					int bytesToWrite = Math.min(compressedData.remaining(), buffer.length);
					compressedData.get(buffer, 0, bytesToWrite);
					write.write(buffer, 0, bytesToWrite);
					writtenBytes += bytesToWrite;
				}
			} catch (Exception e) {
				throw new GdxRuntimeException("Couldn't write PKM file to '" + file + "'", e);
			} finally {
				StreamUtils.closeQuietly(write);
			}
			compressedData.position(dataOffset);
			compressedData.limit(compressedData.capacity());
		}

		
		public void dispose () {
			BufferUtils.disposeUnsafeByteBuffer(compressedData);
		}

		public String toString () {
			if (hasPKMHeader()) {
				return (ETC1.isValidPKM(compressedData, 0) ? "valid" : "invalid") + " pkm [" + ETC1.getWidthPKM(compressedData, 0)
					+ "x" + ETC1.getHeightPKM(compressedData, 0) + "], compressed: "
					+ (compressedData.capacity() - ETC1.PKM_HEADER_SIZE);
			} else {
				return "raw [" + width + "x" + height + "], compressed: " + (compressedData.capacity() - ETC1.PKM_HEADER_SIZE);
			}
		}
	}

	private static int getPixelSize (Format format) {
		if (format == Format.RGB565) return 2;
		if (format == Format.RGB888) return 3;
		throw new GdxRuntimeException("Can only handle RGB565 or RGB888 images");
	}

	
	public static ETC1Data encodeImage (Pixmap pixmap) {
		int pixelSize = getPixelSize(pixmap.getFormat());
		ByteBuffer compressedData = encodeImage(pixmap.getPixels(), 0, pixmap.getWidth(), pixmap.getHeight(), pixelSize);
		BufferUtils.newUnsafeByteBuffer(compressedData);
		return new ETC1Data(pixmap.getWidth(), pixmap.getHeight(), compressedData, 0);
	}

	
	public static ETC1Data encodeImagePKM (Pixmap pixmap) {
		int pixelSize = getPixelSize(pixmap.getFormat());
		ByteBuffer compressedData = encodeImagePKM(pixmap.getPixels(), 0, pixmap.getWidth(), pixmap.getHeight(), pixelSize);
		BufferUtils.newUnsafeByteBuffer(compressedData);
		return new ETC1Data(pixmap.getWidth(), pixmap.getHeight(), compressedData, 16);
	}

	
	public static Pixmap decodeImage (ETC1Data etc1Data, Format format) {
		int dataOffset = 0;
		int width = 0;
		int height = 0;

		if (etc1Data.hasPKMHeader()) {
			dataOffset = 16;
			width = ETC1.getWidthPKM(etc1Data.compressedData, 0);
			height = ETC1.getHeightPKM(etc1Data.compressedData, 0);
		} else {
			dataOffset = 0;
			width = etc1Data.width;
			height = etc1Data.height;
		}

		int pixelSize = getPixelSize(format);
		Pixmap pixmap = new Pixmap(width, height, format);
		decodeImage(etc1Data.compressedData, dataOffset, pixmap.getPixels(), 0, width, height, pixelSize);
		return pixmap;
	}

		

	
	public static native int getCompressedDataSize (int width, int height); 
	

	
	public static native void formatHeader (ByteBuffer header, int offset, int width, int height); 

	
	static native int getWidthPKM (ByteBuffer header, int offset); 

	
	static native int getHeightPKM (ByteBuffer header, int offset); 

	
	static native boolean isValidPKM (ByteBuffer header, int offset); 

	
	private static native void decodeImage (ByteBuffer compressedData, int offset, ByteBuffer decodedData, int offsetDec,
		int width, int height, int pixelSize); 

	
	private static native ByteBuffer encodeImage (ByteBuffer imageData, int offset, int width, int height, int pixelSize); 

	
	private static native ByteBuffer encodeImagePKM (ByteBuffer imageData, int offset, int width, int height, int pixelSize); 
}
