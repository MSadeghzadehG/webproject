

package java.nio;


final class ReadWriteCharArrayBuffer extends CharArrayBuffer {

	static ReadWriteCharArrayBuffer copy (CharArrayBuffer other, int markOfOther) {
		ReadWriteCharArrayBuffer buf = new ReadWriteCharArrayBuffer(other.capacity(), other.backingArray, other.offset);
		buf.limit = other.limit();
		buf.position = other.position();
		buf.mark = markOfOther;
		return buf;
	}

	ReadWriteCharArrayBuffer (char[] array) {
		super(array);
	}

	ReadWriteCharArrayBuffer (int capacity) {
		super(capacity);
	}

	ReadWriteCharArrayBuffer (int capacity, char[] backingArray, int arrayOffset) {
		super(capacity, backingArray, arrayOffset);
	}

	public CharBuffer asReadOnlyBuffer () {
		return ReadOnlyCharArrayBuffer.copy(this, mark);
	}

	public CharBuffer compact () {
		System.arraycopy(backingArray, position + offset, backingArray, offset, remaining());
		position = limit - position;
		limit = capacity;
		mark = UNSET_MARK;
		return this;
	}

	public CharBuffer duplicate () {
		return copy(this, mark);
	}

	public boolean isReadOnly () {
		return false;
	}

	protected char[] protectedArray () {
		return backingArray;
	}

	protected int protectedArrayOffset () {
		return offset;
	}

	protected boolean protectedHasArray () {
		return true;
	}

	public CharBuffer put (char c) {
		if (position == limit) {
			throw new BufferOverflowException();
		}
		backingArray[offset + position++] = c;
		return this;
	}

	public CharBuffer put (int index, char c) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		backingArray[offset + index] = c;
		return this;
	}

	public CharBuffer put (char[] src, int off, int len) {
		int length = src.length;
		if (off < 0 || len < 0 || (long)len + (long)off > length) {
			throw new IndexOutOfBoundsException();
		}
		if (len > remaining()) {
			throw new BufferOverflowException();
		}
		System.arraycopy(src, off, backingArray, offset + position, len);
		position += len;
		return this;
	}

	public CharBuffer slice () {
		return new ReadWriteCharArrayBuffer(remaining(), backingArray, offset + position);
	}

}
