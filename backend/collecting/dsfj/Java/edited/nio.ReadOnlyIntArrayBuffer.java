

package java.nio;


final class ReadOnlyIntArrayBuffer extends IntArrayBuffer {

	static ReadOnlyIntArrayBuffer copy (IntArrayBuffer other, int markOfOther) {
		ReadOnlyIntArrayBuffer buf = new ReadOnlyIntArrayBuffer(other.capacity(), other.backingArray, other.offset);
		buf.limit = other.limit();
		buf.position = other.position();
		buf.mark = markOfOther;
		return buf;
	}

	ReadOnlyIntArrayBuffer (int capacity, int[] backingArray, int arrayOffset) {
		super(capacity, backingArray, arrayOffset);
	}

	public IntBuffer asReadOnlyBuffer () {
		return duplicate();
	}

	public IntBuffer compact () {
		throw new ReadOnlyBufferException();
	}

	public IntBuffer duplicate () {
		return copy(this, mark);
	}

	public boolean isReadOnly () {
		return true;
	}

	protected int[] protectedArray () {
		throw new ReadOnlyBufferException();
	}

	protected int protectedArrayOffset () {
		throw new ReadOnlyBufferException();
	}

	protected boolean protectedHasArray () {
		return false;
	}

	public IntBuffer put (int c) {
		throw new ReadOnlyBufferException();
	}

	public IntBuffer put (int index, int c) {
		throw new ReadOnlyBufferException();
	}

	public IntBuffer put (IntBuffer buf) {
		throw new ReadOnlyBufferException();
	}

	public final IntBuffer put (int[] src, int off, int len) {
		throw new ReadOnlyBufferException();
	}

	public IntBuffer slice () {
		return new ReadOnlyIntArrayBuffer(remaining(), backingArray, offset + position);
	}

}
