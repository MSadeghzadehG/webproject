

package java.nio;


final class ReadWriteFloatArrayBuffer extends FloatArrayBuffer {

	static ReadWriteFloatArrayBuffer copy (FloatArrayBuffer other, int markOfOther) {
		ReadWriteFloatArrayBuffer buf = new ReadWriteFloatArrayBuffer(other.capacity(), other.backingArray, other.offset);
		buf.limit = other.limit();
		buf.position = other.position();
		buf.mark = markOfOther;
		return buf;
	}

	ReadWriteFloatArrayBuffer (float[] array) {
		super(array);
	}

	ReadWriteFloatArrayBuffer (int capacity) {
		super(capacity);
	}

	ReadWriteFloatArrayBuffer (int capacity, float[] backingArray, int arrayOffset) {
		super(capacity, backingArray, arrayOffset);
	}

	public FloatBuffer asReadOnlyBuffer () {
		return ReadOnlyFloatArrayBuffer.copy(this, mark);
	}

	public FloatBuffer compact () {
		for (int i = position + offset, j = offset, k = 0; k < remaining(); i++, j++, k++) {
			backingArray[j] = backingArray[i];
		}
		position = limit - position;
		limit = capacity;
		mark = UNSET_MARK;
		return this;
	}

	public FloatBuffer duplicate () {
		return copy(this, mark);
	}

	public boolean isReadOnly () {
		return false;
	}

	protected float[] protectedArray () {
		return backingArray;
	}

	protected int protectedArrayOffset () {
		return offset;
	}

	protected boolean protectedHasArray () {
		return true;
	}

	public FloatBuffer put (float c) {
		if (position == limit) {
			throw new BufferOverflowException();
		}
		backingArray[offset + position++] = c;
		return this;
	}

	public FloatBuffer put (int index, float c) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		backingArray[offset + index] = c;
		return this;
	}

	public FloatBuffer put (float[] src, int off, int len) {
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

	public FloatBuffer slice () {
		return new ReadWriteFloatArrayBuffer(remaining(), backingArray, offset + position);
	}

}
