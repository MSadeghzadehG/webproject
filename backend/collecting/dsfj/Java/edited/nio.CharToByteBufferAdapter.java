

package java.nio;



final class CharToByteBufferAdapter extends CharBuffer { 
	static CharBuffer wrap (ByteBuffer byteBuffer) {
		return new CharToByteBufferAdapter(byteBuffer.slice());
	}

	private final ByteBuffer byteBuffer;

	CharToByteBufferAdapter (ByteBuffer byteBuffer) {
		super((byteBuffer.capacity() >> 1));
		this.byteBuffer = byteBuffer;
		this.byteBuffer.clear();
	}


	@Override
	public CharBuffer asReadOnlyBuffer () {
		CharToByteBufferAdapter buf = new CharToByteBufferAdapter(byteBuffer.asReadOnlyBuffer());
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public CharBuffer compact () {
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
	public CharBuffer duplicate () {
		CharToByteBufferAdapter buf = new CharToByteBufferAdapter(byteBuffer.duplicate());
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public char get () {
		if (position == limit) {
			throw new BufferUnderflowException();
		}
		return byteBuffer.getChar(position++ << 1);
	}

	@Override
	public char get (int index) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		return byteBuffer.getChar(index << 1);
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
	protected char[] protectedArray () {
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
	public CharBuffer put (char c) {
		if (position == limit) {
			throw new BufferOverflowException();
		}
		byteBuffer.putChar(position++ << 1, c);
		return this;
	}

	@Override
	public CharBuffer put (int index, char c) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		byteBuffer.putChar(index << 1, c);
		return this;
	}

	@Override
	public CharBuffer slice () {
		byteBuffer.limit(limit << 1);
		byteBuffer.position(position << 1);
		CharBuffer result = new CharToByteBufferAdapter(byteBuffer.slice());
		byteBuffer.clear();
		return result;
	}

	@Override
	public CharSequence subSequence (int start, int end) {
		if (start < 0 || end < start || end > remaining()) {
			throw new IndexOutOfBoundsException();
		}

		CharBuffer result = duplicate();
		result.limit(position + end);
		result.position(position + start);
		return result;
	}
}
