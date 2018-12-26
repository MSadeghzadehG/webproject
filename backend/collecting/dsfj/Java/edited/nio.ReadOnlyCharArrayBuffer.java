

package java.nio;


final class ReadOnlyCharArrayBuffer extends CharArrayBuffer {

	static ReadOnlyCharArrayBuffer copy (CharArrayBuffer other, int markOfOther) {
		ReadOnlyCharArrayBuffer buf = new ReadOnlyCharArrayBuffer(other.capacity(), other.backingArray, other.offset);
		buf.limit = other.limit();
		buf.position = other.position();
		buf.mark = markOfOther;
		return buf;
	}

	ReadOnlyCharArrayBuffer (int capacity, char[] backingArray, int arrayOffset) {
		super(capacity, backingArray, arrayOffset);
	}

	public CharBuffer asReadOnlyBuffer () {
		return duplicate();
	}

	public CharBuffer compact () {
		throw new ReadOnlyBufferException();
	}

	public CharBuffer duplicate () {
		return copy(this, mark);
	}

	public boolean isReadOnly () {
		return true;
	}

	protected char[] protectedArray () {
		throw new ReadOnlyBufferException();
	}

	protected int protectedArrayOffset () {
		throw new ReadOnlyBufferException();
	}

	protected boolean protectedHasArray () {
		return false;
	}

	public CharBuffer put (char c) {
		throw new ReadOnlyBufferException();
	}

	public CharBuffer put (int index, char c) {
		throw new ReadOnlyBufferException();
	}

	public final CharBuffer put (char[] src, int off, int len) {
		throw new ReadOnlyBufferException();
	}

	public final CharBuffer put (CharBuffer src) {
		throw new ReadOnlyBufferException();
	}

	public CharBuffer put (String src, int start, int end) {
		if ((start < 0) || (end < 0) || (long)start + (long)end > src.length()) {
			throw new IndexOutOfBoundsException();
		}
		throw new ReadOnlyBufferException();
	}

	public CharBuffer slice () {
		return new ReadOnlyCharArrayBuffer(remaining(), backingArray, offset + position);
	}
}
