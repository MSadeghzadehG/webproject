

package java.nio;

import com.google.gwt.corp.compatibility.Numbers;
import com.google.gwt.typedarrays.shared.ArrayBuffer;


public final class DirectReadWriteByteBuffer extends DirectByteBuffer {

	static DirectReadWriteByteBuffer copy (DirectByteBuffer other, int markOfOther) {
		DirectReadWriteByteBuffer buf = new DirectReadWriteByteBuffer(other.byteArray.buffer(), other.capacity(),
			other.byteArray.byteOffset());
		buf.limit = other.limit();
		buf.position = other.position();
		buf.mark = markOfOther;
		buf.order(other.order());
		return buf;
	}

	DirectReadWriteByteBuffer (ArrayBuffer backingArray) {
		super(backingArray);
	}

	public DirectReadWriteByteBuffer (int capacity) {
		super(capacity);
	}

	DirectReadWriteByteBuffer (ArrayBuffer backingArray, int capacity, int arrayOffset) {
		super(backingArray, capacity, arrayOffset);
	}

	public FloatBuffer asFloatBuffer () {
		return DirectReadWriteFloatBufferAdapter.wrap(this);
	}

	public IntBuffer asIntBuffer () {
		return order() == ByteOrder.nativeOrder() ? DirectReadWriteIntBufferAdapter.wrap(this) : super.asIntBuffer();
	}

	public ShortBuffer asShortBuffer () {
		return order() == ByteOrder.nativeOrder() ? DirectReadWriteShortBufferAdapter.wrap(this) : super.asShortBuffer();
	}

	public ByteBuffer asReadOnlyBuffer () {
		return DirectReadOnlyByteBuffer.copy(this, mark);
	}

	public ByteBuffer compact () {

		int rem = remaining();
		for (int i = 0; i < rem; i++) {
			byteArray.set(i, byteArray.get(position + i));
		}

		position = limit - position;
		limit = capacity;
		mark = UNSET_MARK;
		return this;
	}

	public ByteBuffer duplicate () {
		return copy(this, mark);
	}

	public boolean isReadOnly () {
		return false;
	}

	protected byte[] protectedArray () {
		throw new UnsupportedOperationException();
	}

	protected int protectedArrayOffset () {
		throw new UnsupportedOperationException();
	}

	protected boolean protectedHasArray () {
		return true;
	}

	public ByteBuffer put (byte b) {
		byteArray.set(position++, b);
		return this;
	}

	public ByteBuffer put (int index, byte b) {
		byteArray.set(index, b);
		return this;
	}

	
	public ByteBuffer put (byte[] src, int off, int len) {
		if (off < 0 || len < 0 || (long)off + (long)len > src.length) {
			throw new IndexOutOfBoundsException();
		}
		if (len > remaining()) {
			throw new BufferOverflowException();
		}
		if (isReadOnly()) {
			throw new ReadOnlyBufferException();
		}
		for (int i = 0; i < len; i++) {
			byteArray.set(i + position, src[off + i]);
		}
		position += len;
		return this;
	}

	public ByteBuffer putDouble (double value) {
		return putLong(Numbers.doubleToRawLongBits(value));
	}

	public ByteBuffer putDouble (int index, double value) {
		return putLong(index, Numbers.doubleToRawLongBits(value));
	}

	public ByteBuffer putFloat (float value) {
		return putInt(Numbers.floatToIntBits(value));
	}

	public ByteBuffer putFloat (int index, float value) {
		return putInt(index, Numbers.floatToIntBits(value));
	}

	public ByteBuffer putInt (int value) {
		int newPosition = position + 4;
		store(position, value);
		position = newPosition;
		return this;
	}

	public ByteBuffer putInt (int index, int value) {
		store(index, value);
		return this;
	}

	public ByteBuffer putLong (int index, long value) {
		store(index, value);
		return this;
	}

	public ByteBuffer putLong (long value) {
		int newPosition = position + 8;
		store(position, value);
		position = newPosition;
		return this;
	}

	public ByteBuffer putShort (int index, short value) {
		store(index, value);
		return this;
	}

	public ByteBuffer putShort (short value) {
		int newPosition = position + 2;
		store(position, value);
		position = newPosition;
		return this;
	}

	public ByteBuffer slice () {
		DirectReadWriteByteBuffer slice = new DirectReadWriteByteBuffer(byteArray.buffer(), remaining(),
			byteArray.byteOffset() + position);
		slice.order = order;
		return slice;
	}
}
