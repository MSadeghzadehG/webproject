

package java.nio;


final class ReadOnlyFloatArrayBuffer extends FloatArrayBuffer {

	static ReadOnlyFloatArrayBuffer copy (FloatArrayBuffer other, int markOfOther) {
		ReadOnlyFloatArrayBuffer buf = new ReadOnlyFloatArrayBuffer(other.capacity(), other.backingArray, other.offset);
		buf.limit = other.limit();
		buf.position = other.position();
		buf.mark = markOfOther;
		return buf;
	}

	ReadOnlyFloatArrayBuffer (int capacity, float[] backingArray, int arrayOffset) {
		super(capacity, backingArray, arrayOffset);
	}

	public FloatBuffer asReadOnlyBuffer () {
		return duplicate();
	}

	public FloatBuffer compact () {
		throw new ReadOnlyBufferException();
	}

	public FloatBuffer duplicate () {
		return copy(this, mark);
	}

	public boolean isReadOnly () {
		return true;
	}

	protected float[] protectedArray () {
		throw new ReadOnlyBufferException();
	}

	protected int protectedArrayOffset () {
		throw new ReadOnlyBufferException();
	}

	protected boolean protectedHasArray () {
		return false;
	}

	public FloatBuffer put (float c) {
		throw new ReadOnlyBufferException();
	}

	public FloatBuffer put (int index, float c) {
		throw new ReadOnlyBufferException();
	}

	public FloatBuffer put (FloatBuffer buf) {
		throw new ReadOnlyBufferException();
	}

	public final FloatBuffer put (float[] src, int off, int len) {
		throw new ReadOnlyBufferException();
	}

	public FloatBuffer slice () {
		return new ReadOnlyFloatArrayBuffer(remaining(), backingArray, offset + position);
	}

}
