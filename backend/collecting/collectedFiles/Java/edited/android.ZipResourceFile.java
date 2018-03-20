
package com.badlogic.gdx.backends.android;

import android.content.res.AssetFileDescriptor;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import java.io.EOFException;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.util.Collection;
import java.util.HashMap;
import java.util.Vector;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class ZipResourceFile {

				static final String LOG_TAG = "zipro";
	static final boolean LOGV = false;

		static private int swapEndian(int i) {
		return ((i & 0xff) << 24) + ((i & 0xff00) << 8)
				+ ((i & 0xff0000) >>> 8) + ((i >>> 24) & 0xff);
	}

		static private int swapEndian(short i) {
		return ((i & 0x00FF) << 8 | (i & 0xFF00) >>> 8);
	}

	
	static final int kEOCDSignature = 0x06054b50;
	static final int kEOCDLen = 22;
	static final int kEOCDNumEntries = 8; 	static final int kEOCDSize = 12; 	static final int kEOCDFileOffset = 16; 
	static final int kMaxCommentLen = 65535; 	static final int kMaxEOCDSearch = (kMaxCommentLen + kEOCDLen);

	static final int kLFHSignature = 0x04034b50;
	static final int kLFHLen = 30; 	static final int kLFHNameLen = 26; 	static final int kLFHExtraLen = 28; 
	static final int kCDESignature = 0x02014b50;
	static final int kCDELen = 46; 	static final int kCDEMethod = 10; 	static final int kCDEModWhen = 12; 	static final int kCDECRC = 16; 	static final int kCDECompLen = 20; 	static final int kCDEUncompLen = 24; 	static final int kCDENameLen = 28; 	static final int kCDEExtraLen = 30; 	static final int kCDECommentLen = 32; 	static final int kCDELocalOffset = 42; 
	static final int kCompressStored = 0; 	static final int kCompressDeflated = 8; 
	
	static final int kZipEntryAdj = 10000;

	static public final class ZipEntryRO {
		public ZipEntryRO(final String zipFileName, final File file,
				final String fileName) {
			mFileName = fileName;
			mZipFileName = zipFileName;
			mFile = file;
		}

		public final File mFile;
		public final String mFileName;
		public final String mZipFileName;
		public long mLocalHdrOffset; 
		
		public int mMethod;
		public long mWhenModified;
		public long mCRC32;
		public long mCompressedLength;
		public long mUncompressedLength;

		public long mOffset = -1;

		public void setOffsetFromFile(RandomAccessFile f, ByteBuffer buf)
				throws IOException {
			long localHdrOffset = mLocalHdrOffset;
			try {
				f.seek(localHdrOffset);
				f.readFully(buf.array());
				if (buf.getInt(0) != kLFHSignature) {
					Log.w(LOG_TAG, "didn't find signature at start of lfh");
					throw new IOException();
				}
				int nameLen = buf.getShort(kLFHNameLen) & 0xFFFF;
				int extraLen = buf.getShort(kLFHExtraLen) & 0xFFFF;
				mOffset = localHdrOffset + kLFHLen + nameLen + extraLen;
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}

		
		public long getOffset() {
			return mOffset;
		}

		
		public boolean isUncompressed() {
			return mMethod == kCompressStored;
		}

		public AssetFileDescriptor getAssetFileDescriptor() {
			if (mMethod == kCompressStored) {
				ParcelFileDescriptor pfd;
				try {
					pfd = ParcelFileDescriptor.open(mFile,
							ParcelFileDescriptor.MODE_READ_ONLY);
					return new AssetFileDescriptor(pfd, getOffset(),
							mUncompressedLength);
				} catch (FileNotFoundException e) {
										e.printStackTrace();
				}
			}
			return null;
		}

		public String getZipFileName() {
			return mZipFileName;
		}

		public File getZipFile() {
			return mFile;
		}

	}

	private HashMap<String, ZipEntryRO> mHashMap = new HashMap<String, ZipEntryRO>();

	
	public HashMap<File, ZipFile> mZipFiles = new HashMap<File, ZipFile>();

	public ZipResourceFile(String zipFileName) throws IOException {
		addPatchFile(zipFileName);
	}

	ZipEntryRO[] getEntriesAt(String path) {
		Vector<ZipEntryRO> zev = new Vector<ZipEntryRO>();
		Collection<ZipEntryRO> values = mHashMap.values();
		if (null == path)
			path = "";
		int length = path.length();
		for (ZipEntryRO ze : values) {
			if (ze.mFileName.startsWith(path)) {
				if (-1 == ze.mFileName.indexOf('/', length)) {
					zev.add(ze);
				}
			}
		}
		ZipEntryRO[] entries = new ZipEntryRO[zev.size()];
		return zev.toArray(entries);
	}

	public ZipEntryRO[] getAllEntries() {
		Collection<ZipEntryRO> values = mHashMap.values();
		return values.toArray(new ZipEntryRO[values.size()]);
	}

	
	public AssetFileDescriptor getAssetFileDescriptor(String assetPath) {
		ZipEntryRO entry = mHashMap.get(assetPath);
		if (null != entry) {
			return entry.getAssetFileDescriptor();
		}
		return null;
	}

	
	public InputStream getInputStream(String assetPath) throws IOException {
		ZipEntryRO entry = mHashMap.get(assetPath);
		if (null != entry) {
			if (entry.isUncompressed()) {
				return entry.getAssetFileDescriptor().createInputStream();
			} else {
				ZipFile zf = mZipFiles.get(entry.getZipFile());
				
				if (null == zf) {
					zf = new ZipFile(entry.getZipFile(), ZipFile.OPEN_READ);
					mZipFiles.put(entry.getZipFile(), zf);
				}
				ZipEntry zi = zf.getEntry(assetPath);
				if (null != zi)
					return zf.getInputStream(zi);
			}
		}
		return null;
	}

	ByteBuffer mLEByteBuffer = ByteBuffer.allocate(4);

	static private int read4LE(RandomAccessFile f) throws EOFException,
			IOException {
		return swapEndian(f.readInt());
	}

	
	void addPatchFile(String zipFileName) throws IOException {
		File file = new File(zipFileName);
		RandomAccessFile f = new RandomAccessFile(file, "r");
		long fileLength = f.length();

		if (fileLength < kEOCDLen) {
			f.close();
			throw new java.io.IOException();
		}

		long readAmount = kMaxEOCDSearch;
		if (readAmount > fileLength)
			readAmount = fileLength;

		
		f.seek(0);

		int header = read4LE(f);
		if (header == kEOCDSignature) {
			Log.i(LOG_TAG, "Found Zip archive, but it looks empty");
			throw new IOException();
		} else if (header != kLFHSignature) {
			Log.v(LOG_TAG, "Not a Zip archive");
			throw new IOException();
		}

		
		long searchStart = fileLength - readAmount;

		f.seek(searchStart);
		ByteBuffer bbuf = ByteBuffer.allocate((int) readAmount);
		byte[] buffer = bbuf.array();
		f.readFully(buffer);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);

		

				int eocdIdx;
		for (eocdIdx = buffer.length - kEOCDLen; eocdIdx >= 0; eocdIdx--) {
			if (buffer[eocdIdx] == 0x50
					&& bbuf.getInt(eocdIdx) == kEOCDSignature) {
				if (LOGV) {
					Log.v(LOG_TAG, "+++ Found EOCD at index: " + eocdIdx);
				}
				break;
			}
		}

		if (eocdIdx < 0) {
			Log.d(LOG_TAG, "Zip: EOCD not found, " + zipFileName
					+ " is not zip");
		}

		

		int numEntries = bbuf.getShort(eocdIdx + kEOCDNumEntries);
		long dirSize = bbuf.getInt(eocdIdx + kEOCDSize) & 0xffffffffL;
		long dirOffset = bbuf.getInt(eocdIdx + kEOCDFileOffset) & 0xffffffffL;

				if (dirOffset + dirSize > fileLength) {
			Log.w(LOG_TAG, "bad offsets (dir " + dirOffset + ", size "
					+ dirSize + ", eocd " + eocdIdx + ")");
			throw new IOException();
		}
		if (numEntries == 0) {
			Log.w(LOG_TAG, "empty archive?");
			throw new IOException();
		}

		if (LOGV) {
			Log.v(LOG_TAG, "+++ numEntries=" + numEntries + " dirSize="
					+ dirSize + " dirOffset=" + dirOffset);
		}

		MappedByteBuffer directoryMap = f.getChannel().map(
				FileChannel.MapMode.READ_ONLY, dirOffset, dirSize);
		directoryMap.order(ByteOrder.LITTLE_ENDIAN);

		byte[] tempBuf = new byte[0xffff];

		

		int currentOffset = 0;

		
		ByteBuffer buf = ByteBuffer.allocate(kLFHLen);
		buf.order(ByteOrder.LITTLE_ENDIAN);

		for (int i = 0; i < numEntries; i++) {
			if (directoryMap.getInt(currentOffset) != kCDESignature) {
				Log.w(LOG_TAG, "Missed a central dir sig (at " + currentOffset
						+ ")");
				throw new IOException();
			}

			
			int fileNameLen = directoryMap
					.getShort(currentOffset + kCDENameLen) & 0xffff;
			int extraLen = directoryMap.getShort(currentOffset + kCDEExtraLen) & 0xffff;
			int commentLen = directoryMap.getShort(currentOffset
					+ kCDECommentLen) & 0xffff;

			

			directoryMap.position(currentOffset + kCDELen);
			directoryMap.get(tempBuf, 0, fileNameLen);
			directoryMap.position(0);

			
			String str = new String(tempBuf, 0, fileNameLen);
			if (LOGV) {
				Log.v(LOG_TAG, "Filename: " + str);
			}

			ZipEntryRO ze = new ZipEntryRO(zipFileName, file, str);
			ze.mMethod = directoryMap.getShort(currentOffset + kCDEMethod) & 0xffff;
			ze.mWhenModified = directoryMap.getInt(currentOffset + kCDEModWhen) & 0xffffffffL;
			ze.mCRC32 = directoryMap.getLong(currentOffset + kCDECRC) & 0xffffffffL;
			ze.mCompressedLength = directoryMap.getLong(currentOffset
					+ kCDECompLen) & 0xffffffffL;
			ze.mUncompressedLength = directoryMap.getLong(currentOffset
					+ kCDEUncompLen) & 0xffffffffL;
			ze.mLocalHdrOffset = directoryMap.getInt(currentOffset
					+ kCDELocalOffset) & 0xffffffffL;

						buf.clear();
			ze.setOffsetFromFile(f, buf);

						mHashMap.put(str, ze);

						currentOffset += kCDELen + fileNameLen + extraLen + commentLen;
		}
		if (LOGV) {
			Log.v(LOG_TAG, "+++ zip good scan " + numEntries + " entries");
		}
	}
}