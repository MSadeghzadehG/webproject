

package java.nio;

import com.google.gwt.typedarrays.shared.ArrayBuffer;


final class DirectReadOnlyByteBuffer extends DirectByteBuffer {

	static DirectReadOnlyByteBuffer copy (DirectByteBuffer other, int markOfOther) {
		DirectReadOnlyByteBuffer buf = new DirectReadOnlyByteBuffer(other.byteArray.buffer(), other.capacity(),
			other.byteArray.byteOffset());
		buf.limit = other.limit();
		buf.position = other.position();
		buf.mark = markOfOther;
		buf.order(other.order());
		return buf;
	}

	DirectReadOnlyByteBuffer (ArrayBuffer backingArray, int capacity, int arrayOffset) {
		super(backingArray, capacity, arrayOffset);
	}

	public ByteBuffer asReadOnlyBuffer () {
		return copy(this, mark);
	}

	public ByteBuffer compact () {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer duplicate () {
		return copy(this, mark);
	}

	public boolean isReadOnly () {
		return true;
	}

	protected byte[] protectedArray () {
		throw new ReadOnlyBufferException();
	}

	protected int protectedArrayOffset () {
		throw new ReadOnlyBufferException();
	}

	protected boolean protectedHasArray () {
		return false;
	}

	public FloatBuffer asFloatBuffer () {
		return DirectReadOnlyFloatBufferAdapter.wrap(this);
	}

	public IntBuffer asIntBuffer () {
		return order() == ByteOrder.nativeOrder() ? DirectReadOnlyIntBufferAdapter.wrap(this) : super.asIntBuffer();
	}

	public ShortBuffer asShortBuffer () {
		return order() == ByteOrder.nativeOrder() ? DirectReadOnlyShortBufferAdapter.wrap(this) : super.asShortBuffer();
	}

	public ByteBuffer put (byte b) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer put (int index, byte b) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer put (byte[] src, int off, int len) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer putDouble (double value) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer putDouble (int index, double value) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer putFloat (float value) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer putFloat (int index, float value) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer putInt (int value) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer putInt (int index, int value) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer putLong (int index, long value) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer putLong (long value) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer putShort (int index, short value) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer putShort (short value) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer put (ByteBuffer buf) {
		throw new ReadOnlyBufferException();
	}

	public ByteBuffer slice () {
		DirectReadOnlyByteBuffer slice = new DirectReadOnlyByteBuffer(byteArray.buffer(), remaining(), byteArray.byteOffset()
			+ position);
		slice.order = order;
		return slice;
	}
}
