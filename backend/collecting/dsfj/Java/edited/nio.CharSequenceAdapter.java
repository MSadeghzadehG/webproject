

package java.nio;


final class CharSequenceAdapter extends CharBuffer {

	static CharSequenceAdapter copy (CharSequenceAdapter other) {
		CharSequenceAdapter buf = new CharSequenceAdapter(other.sequence);
		buf.limit = other.limit;
		buf.position = other.position;
		buf.mark = other.mark;
		return buf;
	}

	final CharSequence sequence;

	CharSequenceAdapter (CharSequence chseq) {
		super(chseq.length());
		sequence = chseq;
	}

	public CharBuffer asReadOnlyBuffer () {
		return duplicate();
	}

	public CharBuffer compact () {
		throw new ReadOnlyBufferException();
	}

	public CharBuffer duplicate () {
		return copy(this);
	}

	public char get () {
		if (position == limit) {
			throw new BufferUnderflowException();
		}
		return sequence.charAt(position++);
	}

	public char get (int index) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		return sequence.charAt(index);
	}

	public final CharBuffer get (char[] dest, int off, int len) {
		int length = dest.length;
		if ((off < 0) || (len < 0) || (long)off + (long)len > length) {
			throw new IndexOutOfBoundsException();
		}
		if (len > remaining()) {
			throw new BufferUnderflowException();
		}
		int newPosition = position + len;
		sequence.toString().getChars(position, newPosition, dest, off);
		position = newPosition;
		return this;
	}

	public boolean isDirect () {
		return false;
	}

	public boolean isReadOnly () {
		return true;
	}

	public ByteOrder order () {
		return ByteOrder.nativeOrder();
	}

	protected char[] protectedArray () {
		throw new UnsupportedOperationException();
	}

	protected int protectedArrayOffset () {
		throw new UnsupportedOperationException();
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
		if ((off < 0) || (len < 0) || (long)off + (long)len > src.length) {
			throw new IndexOutOfBoundsException();
		}

		if (len > remaining()) {
			throw new BufferOverflowException();
		}

		throw new ReadOnlyBufferException();
	}

	public CharBuffer put (String src, int start, int end) {
		if ((start < 0) || (end < 0) || (long)start + (long)end > src.length()) {
			throw new IndexOutOfBoundsException();
		}
		throw new ReadOnlyBufferException();
	}

	public CharBuffer slice () {
		return new CharSequenceAdapter(sequence.subSequence(position, limit));
	}

	public CharSequence subSequence (int start, int end) {
		if (end < start || start < 0 || end > remaining()) {
			throw new IndexOutOfBoundsException();
		}

		CharSequenceAdapter result = copy(this);
		result.position = position + start;
		result.limit = position + end;
		return result;
	}
}
