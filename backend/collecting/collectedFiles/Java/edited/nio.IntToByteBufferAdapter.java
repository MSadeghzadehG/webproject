

package java.nio;



final class IntToByteBufferAdapter extends IntBuffer implements ByteBufferWrapper {

	static IntBuffer wrap (ByteBuffer byteBuffer) {
		return new IntToByteBufferAdapter(byteBuffer.slice());
	}

	private final ByteBuffer byteBuffer;

	IntToByteBufferAdapter (ByteBuffer byteBuffer) {
		super((byteBuffer.capacity() >> 2));
		this.byteBuffer = byteBuffer;
		this.byteBuffer.clear();
	}


	@Override
	public IntBuffer asReadOnlyBuffer () {
		IntToByteBufferAdapter buf = new IntToByteBufferAdapter(byteBuffer.asReadOnlyBuffer());
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public IntBuffer compact () {
		if (byteBuffer.isReadOnly()) {
			throw new ReadOnlyBufferException();
		}
		byteBuffer.limit(limit << 2);
		byteBuffer.position(position << 2);
		byteBuffer.compact();
		byteBuffer.clear();
		position = limit - position;
		limit = capacity;
		mark = UNSET_MARK;
		return this;
	}

	@Override
	public IntBuffer duplicate () {
		IntToByteBufferAdapter buf = new IntToByteBufferAdapter(byteBuffer.duplicate());
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public int get () {
		if (position == limit) {
			throw new BufferUnderflowException();
		}
		return byteBuffer.getInt(position++ << 2);
	}

	@Override
	public int get (int index) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		return byteBuffer.getInt(index << 2);
	}

	@Override
	public boolean isDirect () {
		return byteBuffer.isDirect();
	}

	@Override
	public boolean isReadOnly () {
		return byteBuffer.isReadOnly();
	}

	@Override
	public ByteOrder order () {
		return byteBuffer.order();
	}

	@Override
	protected int[] protectedArray () {
		throw new UnsupportedOperationException();
	}

	@Override
	protected int protectedArrayOffset () {
		throw new UnsupportedOperationException();
	}

	@Override
	protected boolean protectedHasArray () {
		return false;
	}

	@Override
	public IntBuffer put (int c) {
		if (position == limit) {
			throw new BufferOverflowException();
		}
		byteBuffer.putInt(position++ << 2, c);
		return this;
	}

	@Override
	public IntBuffer put (int index, int c) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		byteBuffer.putInt(index << 2, c);
		return this;
	}

	@Override
	public IntBuffer slice () {
		byteBuffer.limit(limit << 2);
		byteBuffer.position(position << 2);
		IntBuffer result = new IntToByteBufferAdapter(byteBuffer.slice());
		byteBuffer.clear();
		return result;
	}

	public ByteBuffer getByteBuffer () {
		return byteBuffer;
	}

}
