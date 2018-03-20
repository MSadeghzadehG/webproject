

package jadx.core.utils.android;

import java.io.DataInput;
import java.io.IOException;


public abstract class DataInputDelegate implements DataInput {
	protected final DataInput mDelegate;

	public DataInputDelegate(DataInput delegate) {
		this.mDelegate = delegate;
	}

	public int skipBytes(int n) throws IOException {
		return mDelegate.skipBytes(n);
	}

	public int readUnsignedShort() throws IOException {
		return mDelegate.readUnsignedShort();
	}

	public int readUnsignedByte() throws IOException {
		return mDelegate.readUnsignedByte();
	}

	public String readUTF() throws IOException {
		return mDelegate.readUTF();
	}

	public short readShort() throws IOException {
		return mDelegate.readShort();
	}

	public long readLong() throws IOException {
		return mDelegate.readLong();
	}

	public String readLine() throws IOException {
		return mDelegate.readLine();
	}

	public int readInt() throws IOException {
		return mDelegate.readInt();
	}

	public void readFully(byte[] b, int off, int len) throws IOException {
		mDelegate.readFully(b, off, len);
	}

	public void readFully(byte[] b) throws IOException {
		mDelegate.readFully(b);
	}

	public float readFloat() throws IOException {
		return mDelegate.readFloat();
	}

	public double readDouble() throws IOException {
		return mDelegate.readDouble();
	}

	public char readChar() throws IOException {
		return mDelegate.readChar();
	}

	public byte readByte() throws IOException {
		return mDelegate.readByte();
	}

	public boolean readBoolean() throws IOException {
		return mDelegate.readBoolean();
	}
}
