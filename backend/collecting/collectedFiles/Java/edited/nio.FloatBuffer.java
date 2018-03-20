

package java.nio;


public abstract class FloatBuffer extends Buffer implements Comparable<FloatBuffer> {

	
	public static FloatBuffer allocate (int capacity) {
		if (capacity < 0) {
			throw new IllegalArgumentException();
		}
		return BufferFactory.newFloatBuffer(capacity);
	}

	
	public static FloatBuffer wrap (float[] array) {
		return wrap(array, 0, array.length);
	}

	
	public static FloatBuffer wrap (float[] array, int start, int len) {
		if (array == null) {
			throw new NullPointerException();
		}
		if (start < 0 || len < 0 || (long)start + (long)len > array.length) {
			throw new IndexOutOfBoundsException();
		}

		FloatBuffer buf = BufferFactory.newFloatBuffer(array);
		buf.position = start;
		buf.limit = start + len;

		return buf;
	}

	
	FloatBuffer (int capacity) {
		super(capacity);
	}

	
	public final float[] array () {
		return protectedArray();
	}

	
	public final int arrayOffset () {
		return protectedArrayOffset();
	}

	
	public abstract FloatBuffer asReadOnlyBuffer ();

	
	public abstract FloatBuffer compact ();

	
	public int compareTo (FloatBuffer otherBuffer) {
		int compareRemaining = (remaining() < otherBuffer.remaining()) ? remaining() : otherBuffer.remaining();
		int thisPos = position;
		int otherPos = otherBuffer.position;
				float thisFloat, otherFloat;
		while (compareRemaining > 0) {
			thisFloat = get(thisPos);
			otherFloat = otherBuffer.get(otherPos);
						if ((thisFloat != otherFloat) && ((thisFloat == thisFloat) || (otherFloat == otherFloat))) {
				return thisFloat < otherFloat ? -1 : 1;
			}
			thisPos++;
			otherPos++;
			compareRemaining--;
		}
				return remaining() - otherBuffer.remaining();
	}

	
	public abstract FloatBuffer duplicate ();

	
	public boolean equals (Object other) {
		if (!(other instanceof FloatBuffer)) {
			return false;
		}
		FloatBuffer otherBuffer = (FloatBuffer)other;

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

	
	public abstract float get ();

	
	public FloatBuffer get (float[] dest) {
		return get(dest, 0, dest.length);
	}

	
	public FloatBuffer get (float[] dest, int off, int len) {
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

	
	public abstract float get (int index);

	
	public final boolean hasArray () {
		return protectedHasArray();
	}


	
	public abstract boolean isDirect ();

	
	public abstract ByteOrder order ();

	
	abstract float[] protectedArray ();

	
	abstract int protectedArrayOffset ();

	
	abstract boolean protectedHasArray ();

	
	public abstract FloatBuffer put (float f);

	
	public final FloatBuffer put (float[] src) {
		return put(src, 0, src.length);
	}

	
	public FloatBuffer put (float[] src, int off, int len) {
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

	
	public FloatBuffer put (FloatBuffer src) {
		if (src == this) {
			throw new IllegalArgumentException();
		}
		if (src.remaining() > remaining()) {
			throw new BufferOverflowException();
		}
		float[] contents = new float[src.remaining()];
		src.get(contents);
		put(contents);
		return this;
	}

	
	public abstract FloatBuffer put (int index, float f);

	
	public abstract FloatBuffer slice ();

	
	public String toString () {
		StringBuffer buf = new StringBuffer();
		buf.append(getClass().getName());
		buf.append(", status: capacity="); 		buf.append(capacity());
		buf.append(" position="); 		buf.append(position());
		buf.append(" limit="); 		buf.append(limit());
		return buf.toString();
	}
}
