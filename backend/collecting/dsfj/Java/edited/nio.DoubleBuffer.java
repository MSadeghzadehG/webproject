

package java.nio;


public abstract class DoubleBuffer extends Buffer implements Comparable<DoubleBuffer> {

	
	public static DoubleBuffer allocate (int capacity) {
		if (capacity < 0) {
			throw new IllegalArgumentException();
		}
		return BufferFactory.newDoubleBuffer(capacity);
	}

	
	public static DoubleBuffer wrap (double[] array) {
		return wrap(array, 0, array.length);
	}

	
	public static DoubleBuffer wrap (double[] array, int start, int len) {
		int length = array.length;
		if (start < 0 || len < 0 || (long)start + (long)len > length) {
			throw new IndexOutOfBoundsException();
		}

		DoubleBuffer buf = BufferFactory.newDoubleBuffer(array);
		buf.position = start;
		buf.limit = start + len;

		return buf;
	}

	
	DoubleBuffer (int capacity) {
		super(capacity);
	}

	
	public final double[] array () {
		return protectedArray();
	}

	
	public final int arrayOffset () {
		return protectedArrayOffset();
	}

	
	public abstract DoubleBuffer asReadOnlyBuffer ();

	
	public abstract DoubleBuffer compact ();

	
	public int compareTo (DoubleBuffer otherBuffer) {
		int compareRemaining = (remaining() < otherBuffer.remaining()) ? remaining() : otherBuffer.remaining();
		int thisPos = position;
		int otherPos = otherBuffer.position;
				double thisDouble, otherDouble;
		while (compareRemaining > 0) {
			thisDouble = get(thisPos);
			otherDouble = otherBuffer.get(otherPos);
						if ((thisDouble != otherDouble) && ((thisDouble == thisDouble) || (otherDouble == otherDouble))) {
				return thisDouble < otherDouble ? -1 : 1;
			}
			thisPos++;
			otherPos++;
			compareRemaining--;
		}
				return remaining() - otherBuffer.remaining();
	}

	
	public abstract DoubleBuffer duplicate ();

	
	public boolean equals (Object other) {
		if (!(other instanceof DoubleBuffer)) {
			return false;
		}
		DoubleBuffer otherBuffer = (DoubleBuffer)other;

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

	
	public abstract double get ();

	
	public DoubleBuffer get (double[] dest) {
		return get(dest, 0, dest.length);
	}

	
	public DoubleBuffer get (double[] dest, int off, int len) {
		int length = dest.length;
		if (off < 0 || len < 0 || (long)off + (long)len > length) {
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

	
	public abstract double get (int index);

	
	public final boolean hasArray () {
		return protectedHasArray();
	}


	
	public abstract boolean isDirect ();

	
	public abstract ByteOrder order ();

	
	abstract double[] protectedArray ();

	
	abstract int protectedArrayOffset ();

	
	abstract boolean protectedHasArray ();

	
	public abstract DoubleBuffer put (double d);

	
	public final DoubleBuffer put (double[] src) {
		return put(src, 0, src.length);
	}

	
	public DoubleBuffer put (double[] src, int off, int len) {
		int length = src.length;
		if (off < 0 || len < 0 || (long)off + (long)len > length) {
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

	
	public DoubleBuffer put (DoubleBuffer src) {
		if (src == this) {
			throw new IllegalArgumentException();
		}
		if (src.remaining() > remaining()) {
			throw new BufferOverflowException();
		}
		double[] doubles = new double[src.remaining()];
		src.get(doubles);
		put(doubles);
		return this;
	}

	
	public abstract DoubleBuffer put (int index, double d);

	
	public abstract DoubleBuffer slice ();

	
	public String toString () {
		StringBuffer buf = new StringBuffer();
		buf.append(getClass().getName());
		buf.append(", status: capacity="); 		buf.append(capacity());
		buf.append(" position="); 		buf.append(position());
		buf.append(" limit="); 		buf.append(limit());
		return buf.toString();
	}
}
