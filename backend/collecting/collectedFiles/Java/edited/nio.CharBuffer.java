

package java.nio;

import java.io.IOException;


public abstract class CharBuffer extends Buffer implements Comparable<CharBuffer>, CharSequence, Appendable {
	
	public static CharBuffer allocate (int capacity) {
		if (capacity < 0) {
			throw new IllegalArgumentException();
		}
		return BufferFactory.newCharBuffer(capacity);
	}

	
	public static CharBuffer wrap (char[] array) {
		return wrap(array, 0, array.length);
	}

	
	public static CharBuffer wrap (char[] array, int start, int len) {
		int length = array.length;
		if ((start < 0) || (len < 0) || (long)start + (long)len > length) {
			throw new IndexOutOfBoundsException();
		}

		CharBuffer buf = BufferFactory.newCharBuffer(array);
		buf.position = start;
		buf.limit = start + len;

		return buf;
	}

	
	public static CharBuffer wrap (CharSequence chseq) {
		return BufferFactory.newCharBuffer(chseq);
	}

	
	public static CharBuffer wrap (CharSequence chseq, int start, int end) {
		if (chseq == null) {
			throw new NullPointerException();
		}
		if (start < 0 || end < start || end > chseq.length()) {
			throw new IndexOutOfBoundsException();
		}

		CharBuffer result = BufferFactory.newCharBuffer(chseq);
		result.position = start;
		result.limit = end;
		return result;
	}

	
	CharBuffer (int capacity) {
		super(capacity);
	}

	
	public final char[] array () {
		return protectedArray();
	}

	
	public final int arrayOffset () {
		return protectedArrayOffset();
	}

	
	public abstract CharBuffer asReadOnlyBuffer ();

	
	public final char charAt (int index) {
		if (index < 0 || index >= remaining()) {
			throw new IndexOutOfBoundsException();
		}
		return get(position + index);
	}

	
	public abstract CharBuffer compact ();

	
	public int compareTo (CharBuffer otherBuffer) {
		int compareRemaining = (remaining() < otherBuffer.remaining()) ? remaining() : otherBuffer.remaining();
		int thisPos = position;
		int otherPos = otherBuffer.position;
		char thisByte, otherByte;
		while (compareRemaining > 0) {
			thisByte = get(thisPos);
			otherByte = otherBuffer.get(otherPos);
			if (thisByte != otherByte) {
				return thisByte < otherByte ? -1 : 1;
			}
			thisPos++;
			otherPos++;
			compareRemaining--;
		}
		return remaining() - otherBuffer.remaining();
	}

	
	public abstract CharBuffer duplicate ();

	
	public boolean equals (Object other) {
		if (!(other instanceof CharBuffer)) {
			return false;
		}
		CharBuffer otherBuffer = (CharBuffer)other;

		if (remaining() != otherBuffer.remaining()) {
			return false;
		}

		int myPosition = position;
		int otherPosition = otherBuffer.position;
		boolean equalSoFar = true;
		while (equalSoFar && (myPosition < limit)) {
			equalSoFar = get(myPosition++) == otherBuffer.get(otherPosition++);
		}

		return equalSoFar;
	}

	
	public abstract char get ();

	
	public CharBuffer get (char[] dest) {
		return get(dest, 0, dest.length);
	}

	
	public CharBuffer get (char[] dest, int off, int len) {
		int length = dest.length;
		if ((off < 0) || (len < 0) || (long)off + (long)len > length) {
			throw new IndexOutOfBoundsException();
		}

		if (len > remaining()) {
			throw new BufferUnderflowException();
		}
		for (int i = off; i < off + len; i++) {
			dest[i] = get();
		}
		return this;
	}

	
	public abstract char get (int index);

	
	public final boolean hasArray () {
		return protectedHasArray();
	}

	
	public int hashCode () {
		int myPosition = position;
		int hash = 0;
		while (myPosition < limit) {
			hash = hash + get(myPosition++);
		}
		return hash;
	}

	
	public abstract boolean isDirect ();

	
	public final int length () {
		return remaining();
	}

	
	public abstract ByteOrder order ();

	
	abstract char[] protectedArray ();

	
	abstract int protectedArrayOffset ();

	
	abstract boolean protectedHasArray ();

	
	public abstract CharBuffer put (char c);

	
	public final CharBuffer put (char[] src) {
		return put(src, 0, src.length);
	}

	
	public CharBuffer put (char[] src, int off, int len) {
		int length = src.length;
		if ((off < 0) || (len < 0) || (long)off + (long)len > length) {
			throw new IndexOutOfBoundsException();
		}

		if (len > remaining()) {
			throw new BufferOverflowException();
		}
		for (int i = off; i < off + len; i++) {
			put(src[i]);
		}
		return this;
	}

	
	public CharBuffer put (CharBuffer src) {
		if (src == this) {
			throw new IllegalArgumentException();
		}
		if (src.remaining() > remaining()) {
			throw new BufferOverflowException();
		}

		char[] contents = new char[src.remaining()];
		src.get(contents);
		put(contents);
		return this;
	}

	
	public abstract CharBuffer put (int index, char c);

	
	public final CharBuffer put (String str) {
		return put(str, 0, str.length());
	}

	
	public CharBuffer put (String str, int start, int end) {
		int length = str.length();
		if (start < 0 || end < start || end > length) {
			throw new IndexOutOfBoundsException();
		}

		if (end - start > remaining()) {
			throw new BufferOverflowException();
		}
		for (int i = start; i < end; i++) {
			put(str.charAt(i));
		}
		return this;
	}

	
	public abstract CharBuffer slice ();

	
	public abstract CharSequence subSequence (int start, int end);

	
	public String toString () {
		StringBuffer strbuf = new StringBuffer();
		for (int i = position; i < limit; i++) {
			strbuf.append(get(i));
		}
		return strbuf.toString();
	}

	
	public CharBuffer append (char c) {
		return put(c);
	}

	
	public CharBuffer append (CharSequence csq) {
		if (csq != null) {
			return put(csq.toString());
		}
		return put("null"); 	}

	
	public CharBuffer append (CharSequence csq, int start, int end) {
		if (csq == null) {
			csq = "null"; 		}
		CharSequence cs = csq.subSequence(start, end);
		if (cs.length() > 0) {
			return put(cs.toString());
		}
		return this;
	}

	
	public int read (CharBuffer target) throws IOException {
		if (target == this) {
			throw new IllegalArgumentException();
		}
		if (remaining() == 0) {
			return target.remaining() == 0 ? 0 : -1;
		}
		int result = Math.min(target.remaining(), remaining());
		char[] chars = new char[result];
		get(chars);
		target.put(chars);
		return result;
	}
}
