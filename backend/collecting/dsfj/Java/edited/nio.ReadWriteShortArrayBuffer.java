

package java.nio;


final class ReadWriteShortArrayBuffer extends ShortArrayBuffer {

	static ReadWriteShortArrayBuffer copy (ShortArrayBuffer other, int markOfOther) {
		ReadWriteShortArrayBuffer buf = new ReadWriteShortArrayBuffer(other.capacity(), other.backingArray, other.offset);
		buf.limit = other.limit();
		buf.position = other.position();
		buf.mark = markOfOther;
		return buf;
	}

	ReadWriteShortArrayBuffer (short[] array) {
		super(array);
	}

	ReadWriteShortArrayBuffer (int capacity) {
		super(capacity);
	}

	ReadWriteShortArrayBuffer (int capacity, short[] backingArray, int arrayOffset) {
		super(capacity, backingArray, arrayOffset);
	}

	public ShortBuffer asReadOnlyBuffer () {
		return ReadOnlyShortArrayBuffer.copy(this, mark);
	}

	public ShortBuffer compact () {
		System.arraycopy(backingArray, position + offset, backingArray, offset, remaining());
		position = limit - position;
		limit = capacity;
		mark = UNSET_MARK;
		return this;
	}

	public ShortBuffer duplicate () {
		return copy(this, mark);
	}

	public boolean isReadOnly () {
		return false;
	}

	protected short[] protectedArray () {
		return backingArray;
	}

	protected int protectedArrayOffset () {
		return offset;
	}

	protected boolean protectedHasArray () {
		return true;
	}

	public ShortBuffer put (short c) {
		if (position == limit) {
			throw new BufferOverflowException();
		}
		backingArray[offset + position++] = c;
		return this;
	}

	public ShortBuffer put (int index, short c) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		backingArray[offset + index] = c;
		return this;
	}

	public ShortBuffer put (short[] src, int off, int len) {
		int length = src.length;
		if (off < 0 || len < 0 || (long)off + (long)len > length) {
			throw new IndexOutOfBoundsException();
		}
		if (len > remaining()) {
			throw new BufferOverflowException();
		}
		System.arraycopy(src, off, backingArray, offset + position, len);
		position += len;
		return this;
	}

	public ShortBuffer slice () {
		return new ReadWriteShortArrayBuffer(remaining(), backingArray, offset + position);
	}

}
