

package java.nio;

import com.google.gwt.corp.compatibility.Endianness;
import com.google.gwt.corp.compatibility.Numbers;
import com.google.gwt.typedarrays.client.ArrayBufferNative;
import com.google.gwt.typedarrays.client.Int8ArrayNative;
import com.google.gwt.typedarrays.shared.ArrayBuffer;
import com.google.gwt.typedarrays.shared.ArrayBufferView;
import com.google.gwt.typedarrays.shared.Int8Array;


abstract class DirectByteBuffer extends BaseByteBuffer implements HasArrayBufferView {

	Int8Array byteArray;

	DirectByteBuffer (int capacity) {
		this(ArrayBufferNative.create(capacity), capacity, 0);
	}

	DirectByteBuffer (ArrayBuffer buf) {
		this(buf, buf.byteLength(), 0);
	}

	DirectByteBuffer (ArrayBuffer buffer, int capacity, int offset) {
		super(capacity);
		byteArray = Int8ArrayNative.create(buffer, offset, capacity);
	}

	public ArrayBufferView getTypedArray () {
		return byteArray;
	}

	public int getElementSize () {
		return 1;
	}

	
	public final ByteBuffer get (byte[] dest, int off, int len) {
		int length = dest.length;
		if (off < 0 || len < 0 || (long)off + (long)len > length) {
			throw new IndexOutOfBoundsException();
		}
		if (len > remaining()) {
			throw new BufferUnderflowException();
		}

		for (int i = 0; i < len; i++) {
			dest[i + off] = get(position + i);
		}

		position += len;
		return this;
	}

	public final byte get () {
		return (byte)byteArray.get(position++);
	}

	public final byte get (int index) {
		return (byte)byteArray.get(index);
	}

	public final double getDouble () {
		return Numbers.longBitsToDouble(getLong());
	}

	public final double getDouble (int index) {
		return Numbers.longBitsToDouble(getLong(index));
	}

	public final float getFloat () {
		return Numbers.intBitsToFloat(getInt());
	}

	public final float getFloat (int index) {
		return Numbers.intBitsToFloat(getInt(index));
	}

	public final int getInt () {
		int newPosition = position + 4;
		int result = loadInt(position);
		position = newPosition;
		return result;
	}

	public final int getInt (int index) {
		return loadInt(index);
	}

	public final long getLong () {
		int newPosition = position + 8;
		long result = loadLong(position);
		position = newPosition;
		return result;
	}

	public final long getLong (int index) {
		return loadLong(index);
	}

	public final short getShort () {
		int newPosition = position + 2;
		short result = loadShort(position);
		position = newPosition;
		return result;
	}

	public final short getShort (int index) {
		return loadShort(index);
	}

	public final boolean isDirect () {
		return false;
	}

	protected final int loadInt (int baseOffset) {
		int bytes = 0;
		if (order == Endianness.BIG_ENDIAN) {
			for (int i = 0; i < 4; i++) {
				bytes = bytes << 8;
				bytes = bytes | (byteArray.get(baseOffset + i) & 0xFF);
			}
		} else {
			for (int i = 3; i >= 0; i--) {
				bytes = bytes << 8;
				bytes = bytes | (byteArray.get(baseOffset + i) & 0xFF);
			}
		}
		return bytes;
	}

	protected final long loadLong (int baseOffset) {
		long bytes = 0;
		if (order == Endianness.BIG_ENDIAN) {
			for (int i = 0; i < 8; i++) {
				bytes = bytes << 8;
				bytes = bytes | (byteArray.get(baseOffset + i) & 0xFF);
			}
		} else {
			for (int i = 7; i >= 0; i--) {
				bytes = bytes << 8;
				bytes = bytes | (byteArray.get(baseOffset + i) & 0xFF);
			}
		}
		return bytes;
	}

	protected final short loadShort (int baseOffset) {
		short bytes = 0;
		if (order == Endianness.BIG_ENDIAN) {
			bytes = (short)(byteArray.get(baseOffset) << 8);
			bytes |= (byteArray.get(baseOffset + 1) & 0xFF);
		} else {
			bytes = (short)(byteArray.get(baseOffset + 1) << 8);
			bytes |= (byteArray.get(baseOffset) & 0xFF);
		}
		return bytes;
	}

	protected final void store (int baseOffset, int value) {
		if (order == Endianness.BIG_ENDIAN) {
			for (int i = 3; i >= 0; i--) {
				byteArray.set(baseOffset + i, (byte)(value & 0xFF));
				value = value >> 8;
			}
		} else {
			for (int i = 0; i <= 3; i++) {
				byteArray.set(baseOffset + i, (byte)(value & 0xFF));
				value = value >> 8;
			}
		}
	}

	protected final void store (int baseOffset, long value) {
		if (order == Endianness.BIG_ENDIAN) {
			for (int i = 7; i >= 0; i--) {
				byteArray.set(baseOffset + i, (byte)(value & 0xFF));
				value = value >> 8;
			}
		} else {
			for (int i = 0; i <= 7; i++) {
				byteArray.set(baseOffset + i, (byte)(value & 0xFF));
				value = value >> 8;
			}
		}
	}

	protected final void store (int baseOffset, short value) {
		if (order == Endianness.BIG_ENDIAN) {
			byteArray.set(baseOffset, (byte)((value >> 8) & 0xFF));
			byteArray.set(baseOffset + 1, (byte)(value & 0xFF));
		} else {
			byteArray.set(baseOffset + 1, (byte)((value >> 8) & 0xFF));
			byteArray.set(baseOffset, (byte)(value & 0xFF));
		}
	}
}
