

package java.nio;


abstract class IntArrayBuffer extends IntBuffer {

	protected final int[] backingArray;

	protected final int offset;

	IntArrayBuffer (int[] array) {
		this(array.length, array, 0);
	}

	IntArrayBuffer (int capacity) {
		this(capacity, new int[capacity], 0);
	}

	IntArrayBuffer (int capacity, int[] backingArray, int offset) {
		super(capacity);
		this.backingArray = backingArray;
		this.offset = offset;
	}

	public final int get () {
		if (position == limit) {
			throw new BufferUnderflowException();
		}
		return backingArray[offset + position++];
	}

	public final int get (int index) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		return backingArray[offset + index];
	}

	public final IntBuffer get (int[] dest, int off, int len) {
		int length = dest.length;
		if (off < 0 || len < 0 || (long)len + (long)off > length) {
			throw new IndexOutOfBoundsException();
		}
		if (len > remaining()) {
			throw new BufferUnderflowException();
		}
		System.arraycopy(backingArray, offset + position, dest, off, len);
		position += len;
		return this;
	}

	public final boolean isDirect () {
		return false;
	}

	public final ByteOrder order () {
		return ByteOrder.nativeOrder();
	}

}
