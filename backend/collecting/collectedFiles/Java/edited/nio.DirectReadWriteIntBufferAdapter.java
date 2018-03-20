

package java.nio;

import com.google.gwt.typedarrays.shared.ArrayBufferView;
import com.google.gwt.typedarrays.shared.Int32Array;
import com.google.gwt.typedarrays.shared.TypedArrays;


final class DirectReadWriteIntBufferAdapter extends IntBuffer implements HasArrayBufferView {

	static IntBuffer wrap (DirectReadWriteByteBuffer byteBuffer) {
		return new DirectReadWriteIntBufferAdapter((DirectReadWriteByteBuffer)byteBuffer.slice());
	}

	private final DirectReadWriteByteBuffer byteBuffer;
	private final Int32Array intArray;

	DirectReadWriteIntBufferAdapter (DirectReadWriteByteBuffer byteBuffer) {
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
		byteBuffer.limit(limit << 2);
		byteBuffer.position(position << 2);
		byteBuffer.compact();
		byteBuffer.clear();
		position = limit - position;
		limit = capacity;
		mark = UNSET_MARK;
		return this;
	}

	@Override
	public IntBuffer duplicate () {
		DirectReadWriteIntBufferAdapter buf = new DirectReadWriteIntBufferAdapter((DirectReadWriteByteBuffer)byteBuffer.duplicate());
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
		return intArray.get(index);
	}

	@Override
	public boolean isDirect () {
		return true;
	}

	@Override
	public boolean isReadOnly () {
		return false;
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
		intArray.set(position++, c);
		return this;
	}

	@Override
	public IntBuffer put (int index, int c) {
		intArray.set(index, c);
		return this;
	}

	@Override
	public IntBuffer slice () {
		byteBuffer.limit(limit << 2);
		byteBuffer.position(position << 2);
		IntBuffer result = new DirectReadWriteIntBufferAdapter((DirectReadWriteByteBuffer)byteBuffer.slice());
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
