

package java.nio;

import com.google.gwt.typedarrays.shared.ArrayBufferView;
import com.google.gwt.typedarrays.shared.Int32Array;
import com.google.gwt.typedarrays.shared.TypedArrays;


final class DirectReadOnlyIntBufferAdapter extends IntBuffer implements HasArrayBufferView {

	static IntBuffer wrap (DirectByteBuffer byteBuffer) {
		return new DirectReadOnlyIntBufferAdapter((DirectByteBuffer)byteBuffer.slice());
	}

	private final DirectByteBuffer byteBuffer;
	private final Int32Array intArray;

	DirectReadOnlyIntBufferAdapter (DirectByteBuffer byteBuffer) {
		super((byteBuffer.capacity() >> 2));
		this.byteBuffer = byteBuffer;
		this.byteBuffer.clear();
		this.intArray = TypedArrays.createInt32Array(byteBuffer.byteArray.buffer(), byteBuffer.byteArray.byteOffset(), capacity);
	}

	@Override
	public IntBuffer asReadOnlyBuffer () {
		DirectReadOnlyIntBufferAdapter buf = new DirectReadOnlyIntBufferAdapter(byteBuffer);
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public IntBuffer compact () {
		throw new ReadOnlyBufferException();
	}

	@Override
	public IntBuffer duplicate () {
		DirectReadOnlyIntBufferAdapter buf = new DirectReadOnlyIntBufferAdapter((DirectByteBuffer)byteBuffer.duplicate());
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public int get () {
		return intArray.get(position++);
	}

	@Override
	public int get (int index) {
		if (index < 0 || index >= limit) {
			throw new IndexOutOfBoundsException();
		}
		return intArray.get(index);
	}

	@Override
	public boolean isDirect () {
		return true;
	}

	@Override
	public boolean isReadOnly () {
		return true;
	}

	@Override
	public ByteOrder order () {
		return byteBuffer.order();
	}

	@Override
	protected int[] protectedArray () {
		throw new UnsupportedOperationException();
	}

	@Override
	protected int protectedArrayOffset () {
		throw new UnsupportedOperationException();
	}

	@Override
	protected boolean protectedHasArray () {
		return false;
	}

	@Override
	public IntBuffer put (int c) {
		throw new ReadOnlyBufferException();
	}

	@Override
	public IntBuffer put (int index, int c) {
		throw new ReadOnlyBufferException();
	}

	@Override
	public IntBuffer slice () {
		byteBuffer.limit(limit << 2);
		byteBuffer.position(position << 2);
		IntBuffer result = new DirectReadOnlyIntBufferAdapter((DirectByteBuffer)byteBuffer.slice());
		byteBuffer.clear();
		return result;
	}

	public ArrayBufferView getTypedArray () {
		return intArray;
	}

	public int getElementSize () {
		return 4;
	}
}
