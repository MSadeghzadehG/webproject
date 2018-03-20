

package java.nio;


final class ReadOnlyShortArrayBuffer extends ShortArrayBuffer {

	static ReadOnlyShortArrayBuffer copy (ShortArrayBuffer other, int markOfOther) {
		ReadOnlyShortArrayBuffer buf = new ReadOnlyShortArrayBuffer(other.capacity(), other.backingArray, other.offset);
		buf.limit = other.limit();
		buf.position = other.position();
		buf.mark = markOfOther;
		return buf;
	}

	ReadOnlyShortArrayBuffer (int capacity, short[] backingArray, int arrayOffset) {
		super(capacity, backingArray, arrayOffset);
	}

	public ShortBuffer asReadOnlyBuffer () {
		return duplicate();
	}

	public ShortBuffer compact () {
		throw new ReadOnlyBufferException();
	}

	public ShortBuffer duplicate () {
		return copy(this, mark);
	}

	public boolean isReadOnly () {
		return true;
	}

	protected short[] protectedArray () {
		throw new ReadOnlyBufferException();
	}

	protected int protectedArrayOffset () {
		throw new ReadOnlyBufferException();
	}

	protected boolean protectedHasArray () {
		return false;
	}

	public ShortBuffer put (ShortBuffer buf) {
		throw new ReadOnlyBufferException();
	}

	public ShortBuffer put (short c) {
		throw new ReadOnlyBufferException();
	}

	public ShortBuffer put (int index, short c) {
		throw new ReadOnlyBufferException();
	}

	public final ShortBuffer put (short[] src, int off, int len) {
		throw new ReadOnlyBufferException();
	}

	public ShortBuffer slice () {
		return new ReadOnlyShortArrayBuffer(remaining(), backingArray, offset + position);
	}

}
