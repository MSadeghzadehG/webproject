

package java.nio;



final class ShortToByteBufferAdapter extends ShortBuffer implements ByteBufferWrapper {

	static ShortBuffer wrap (ByteBuffer byteBuffer) {
		return new ShortToByteBufferAdapter(byteBuffer.slice());
	}

	private final ByteBuffer byteBuffer;

	ShortToByteBufferAdapter (ByteBuffer byteBuffer) {
		super((byteBuffer.capacity() >> 1));
		this.byteBuffer = byteBuffer;
		this.byteBuffer.clear();
	}


	@Override
	public ShortBuffer asReadOnlyBuffer () {
		ShortToByteBufferAdapter buf = new ShortToByteBufferAdapter(byteBuffer.asReadOnlyBuffer());
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public ShortBuffer compact () {
		if (byteBuffer.isReadOnly()) {
			throw new ReadOnlyBufferException();
		}
		byteBuffer.limit(limit << 1);
		byteBuffer.position(position << 1);
		byteBuffer.compact();
		byteBuffer.clear();
		position = limit - position;
		limit = capacity;
		mark = UNSET_MARK;
		return this;
	}

	@Override
	public ShortBuffer duplicate () {
		ShortToByteBufferAdapter buf = new ShortToByteBufferAdapter(byteBuffer.duplicate());
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public short get () {
		if (position == limit) {
			throw new BufferUnderflowException();
		}
		return byteBuffer.getShort(position++ << 1);
	}

	@Override
	public short get (int index) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		return byteBuffer.getShort(index << 1);
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
	protected short[] protectedArray () {
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
	public ShortBuffer put (short c) {
		if (position == limit) {
			throw new BufferOverflowException();
		}
		byteBuffer.putShort(position++ << 1, c);
		return this;
	}

	@Override
	public ShortBuffer put (int index, short c) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		byteBuffer.putShort(index << 1, c);
		return this;
	}

	@Override
	public ShortBuffer slice () {
		byteBuffer.limit(limit << 1);
		byteBuffer.position(position << 1);
		ShortBuffer result = new ShortToByteBufferAdapter(byteBuffer.slice());
		byteBuffer.clear();
		return result;
	}

	public ByteBuffer getByteBuffer () {
		return byteBuffer;
	}

}
