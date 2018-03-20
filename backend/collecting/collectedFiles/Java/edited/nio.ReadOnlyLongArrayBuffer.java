

package java.nio;


final class ReadOnlyLongArrayBuffer extends LongArrayBuffer {

	static ReadOnlyLongArrayBuffer copy (LongArrayBuffer other, int markOfOther) {
		ReadOnlyLongArrayBuffer buf = new ReadOnlyLongArrayBuffer(other.capacity(), other.backingArray, other.offset);
		buf.limit = other.limit();
		buf.position = other.position();
		buf.mark = markOfOther;
		return buf;
	}

	ReadOnlyLongArrayBuffer (int capacity, long[] backingArray, int arrayOffset) {
		super(capacity, backingArray, arrayOffset);
	}

	public LongBuffer asReadOnlyBuffer () {
		return duplicate();
	}

	public LongBuffer compact () {
		throw new ReadOnlyBufferException();
	}

	public LongBuffer duplicate () {
		return copy(this, mark);
	}

	public boolean isReadOnly () {
		return true;
	}

	protected long[] protectedArray () {
		throw new ReadOnlyBufferException();
	}

	protected int protectedArrayOffset () {
		throw new ReadOnlyBufferException();
	}

	protected boolean protectedHasArray () {
		return false;
	}

	public LongBuffer put (long c) {
		throw new ReadOnlyBufferException();
	}

	public LongBuffer put (int index, long c) {
		throw new ReadOnlyBufferException();
	}

	public LongBuffer put (LongBuffer buf) {
		throw new ReadOnlyBufferException();
	}

	public final LongBuffer put (long[] src, int off, int len) {
		throw new ReadOnlyBufferException();
	}

	public LongBuffer slice () {
		return new ReadOnlyLongArrayBuffer(remaining(), backingArray, offset + position);
	}

}
