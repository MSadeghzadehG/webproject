

package java.nio;



final class LongToByteBufferAdapter extends LongBuffer {
	static LongBuffer wrap (ByteBuffer byteBuffer) {
		return new LongToByteBufferAdapter(byteBuffer.slice());
	}

	private final ByteBuffer byteBuffer;

	LongToByteBufferAdapter (ByteBuffer byteBuffer) {
		super((byteBuffer.capacity() >> 3));
		this.byteBuffer = byteBuffer;
		this.byteBuffer.clear();
	}


	@Override
	public LongBuffer asReadOnlyBuffer () {
		LongToByteBufferAdapter buf = new LongToByteBufferAdapter(byteBuffer.asReadOnlyBuffer());
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public LongBuffer compact () {
		if (byteBuffer.isReadOnly()) {
			throw new ReadOnlyBufferException();
		}
		byteBuffer.limit(limit << 3);
		byteBuffer.position(position << 3);
		byteBuffer.compact();
		byteBuffer.clear();
		position = limit - position;
		limit = capacity;
		mark = UNSET_MARK;
		return this;
	}

	@Override
	public LongBuffer duplicate () {
		LongToByteBufferAdapter buf = new LongToByteBufferAdapter(byteBuffer.duplicate());
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public long get () {
		if (position == limit) {
			throw new BufferUnderflowException();
		}
		return byteBuffer.getLong(position++ << 3);
	}

	@Override
	public long get (int index) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		return byteBuffer.getLong(index << 3);
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
	protected long[] protectedArray () {
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
	public LongBuffer put (long c) {
		if (position == limit) {
			throw new BufferOverflowException();
		}
		byteBuffer.putLong(position++ << 3, c);
		return this;
	}

	@Override
	public LongBuffer put (int index, long c) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		byteBuffer.putLong(index << 3, c);
		return this;
	}

	@Override
	public LongBuffer slice () {
		byteBuffer.limit(limit << 3);
		byteBuffer.position(position << 3);
		LongBuffer result = new LongToByteBufferAdapter(byteBuffer.slice());
		byteBuffer.clear();
		return result;
	}

}
