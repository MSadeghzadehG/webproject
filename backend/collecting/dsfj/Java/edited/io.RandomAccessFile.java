

package java.io;

import com.google.gwt.storage.client.Storage;


public class RandomAccessFile {
	

	final String name;
	final boolean writeable;
	boolean dirty;
	String data;
	int newDataPos;
	StringBuilder newData;
	int pos;
	int len;
	
	DataInputStream dis = new DataInputStream(new RafInputStream());
	DataOutputStream dos = new DataOutputStream(new RafOutputStream());

	public RandomAccessFile (String name, String mode) throws FileNotFoundException {
		this(new File(name), mode);
	}

	public RandomAccessFile (File file, String mode) throws FileNotFoundException {
		name = file.getCanonicalPath();

		mode = mode.toLowerCase();
		if (!mode.equals("r") && !mode.equals("rw")) {
			throw new IllegalArgumentException("mode: '" + mode + "'");
		}
		writeable = mode.equals("rw");
		if (file.exists()) {
			data = atob(File.LocalStorage.getItem(name));
			len = data.length();
		} else if (writeable) {
			data = "";
			dirty = true;
			try {
				flush();
			} catch (IOException e) {
				throw new FileNotFoundException("" + e);
			}
		} else {
			throw new FileNotFoundException(name);
		}
	}

	static native String btoa (String s) ;

	static native String atob (String s) ;

	public int read () throws IOException {
		return dis.read();
	}

	public int read (byte b[], int off, int len) throws IOException {
		return dis.read(b, off, len);
	}

	public int read (byte b[]) throws IOException {
		return dis.read(b);
	}

	public final void readFully (byte b[]) throws IOException {
		dis.readFully(b);
	}

	public final void readFully (byte b[], int off, int len) throws IOException {
		dis.readFully(b, off, len);
	}

	public int skipBytes (int n) throws IOException {
		return dis.skipBytes(n);
	}

	public void write (int b) throws IOException {
		dos.write(b);
	};

	public void write (byte b[]) throws IOException {
		dos.write(b);
	}

	public void write (byte b[], int off, int len) throws IOException {
		dos.write(b, off, len);
	}

	public long getFilePointer () throws IOException {
		return pos;
	}

	public void seek (long pos) throws IOException {
		if (pos < 0) {
			throw new IllegalArgumentException();
		}
		this.pos = (int)pos;
	}

	public long length () throws IOException {
		return len;
	}

	public void setLength (long newLength) throws IOException {
		if (len != newLength) {
			consolidate();
			if (data.length() > newLength) {
				data = data.substring(0, (int)newLength);
				len = (int)newLength;
			} else {
				while (len < newLength) {
					write(0);
				}
			}
		}
	}

	public void close () throws IOException {
		if (data != null) {
			flush();
			data = null;
		}
	}

	void consolidate () {
		if (newData == null) {
			return;
		}


		if (data.length() < newDataPos) {
			StringBuilder filler = new StringBuilder();
			while (data.length() + filler.length() < newDataPos) {
				filler.append('\0');
			}

			data += filler.toString();
		}

		int p2 = newDataPos + newData.length();
		data = data.substring(0, newDataPos) + newData.toString() + (p2 < data.length() ? data.substring(p2) : "");
		newData = null;
	}

	void flush () throws IOException {
		if (!dirty) {
			return;
		}
		consolidate();
		File.LocalStorage.setItem(name, btoa(data));
		dirty = false;
	}

	public final boolean readBoolean () throws IOException {
		return dis.readBoolean();
	}

	public final byte readByte () throws IOException {
		return dis.readByte();
	}

	public final int readUnsignedByte () throws IOException {
		return dis.readUnsignedByte();
	}

	public final short readShort () throws IOException {
		return dis.readShort();
	}

	public final int readUnsignedShort () throws IOException {
		return dis.readUnsignedShort();
	}

	public final char readChar () throws IOException {
		return dis.readChar();
	}

	public final int readInt () throws IOException {
		return dis.readInt();
	}

	public final long readLong () throws IOException {
		return dis.readLong();
	}

	public final float readFloat () throws IOException {
		return dis.readFloat();
	}

	public final double readDouble () throws IOException {
		return dis.readDouble();
	}

	public final String readLine () throws IOException {
		return dis.readLine();
	}

	public final String readUTF () throws IOException {
		return dis.readUTF();
	}

	public final void writeBoolean (boolean v) throws IOException {
		dos.writeBoolean(v);
	}

	public final void writeByte (int v) throws IOException {
		dos.writeByte(v);
	}

	public final void writeShort (int v) throws IOException {
		dos.writeShort(v);
	}

	public final void writeChar (int v) throws IOException {
		dos.writeChar(v);
	}

	public final void writeInt (int v) throws IOException {
		dos.writeInt(v);
	}

	public final void writeLong (long v) throws IOException {
		dos.writeLong(v);
	}

	public final void writeFloat (float v) throws IOException {
		dos.writeFloat(v);
	}

	public final void writeDouble (double v) throws IOException {
		dos.writeDouble(v);
	}

	public final void writeBytes (String s) throws IOException {
		dos.writeBytes(s);
	}

	public final void writeChars (String s) throws IOException {
		dos.writeChars(s);
	}

	public final void writeUTF (String str) throws IOException {
		dos.writeUTF(str);
	}


	class RafInputStream extends InputStream {
		@Override
		public int read () throws IOException {
			if (pos >= len) {
				return -1;
			} else {
				consolidate();
				return data.charAt(pos++);
			}
		}
	}

	class RafOutputStream extends OutputStream {

		public void write (int b) throws IOException {
			if (!writeable) {
				throw new IOException("not writeable");
			}

			if (newData == null) {
				newDataPos = pos;
				newData = new StringBuilder();
			} else if (newDataPos + newData.length() != pos) {
				consolidate();
				newDataPos = pos;
				newData = new StringBuilder();
			}

			newData.append((char)(b & 255));
			pos++;
			len = Math.max(pos, len);
			dirty = true;
		}
	}
}
