

package java.nio;


abstract class BaseByteBuffer extends ByteBuffer {

	protected BaseByteBuffer (int capacity) {
		super(capacity);
	}

	@Override
	public CharBuffer asCharBuffer () {
		return CharToByteBufferAdapter.wrap(this);
	}

	@Override
	public DoubleBuffer asDoubleBuffer () {
		return DoubleToByteBufferAdapter.wrap(this);
	}

	@Override
	public FloatBuffer asFloatBuffer () {
		return FloatToByteBufferAdapter.wrap(this);
	}

	@Override
	public IntBuffer asIntBuffer () {
		return IntToByteBufferAdapter.wrap(this);
	}

	@Override
	public LongBuffer asLongBuffer () {
		return LongToByteBufferAdapter.wrap(this);
	}

	@Override
	public ShortBuffer asShortBuffer () {
		return ShortToByteBufferAdapter.wrap(this);
	}

	public final char getChar () {
		return (char)getShort();
	}

	public final char getChar (int index) {
		return (char)getShort(index);
	}

	public final ByteBuffer putChar (char value) {
		return putShort((short)value);
	}

	public final ByteBuffer putChar (int index, char value) {
		return putShort(index, (short)value);
	}
}
