

package java.nio;

import com.google.gwt.typedarrays.shared.ArrayBufferView;
import com.google.gwt.typedarrays.shared.Float32Array;
import com.google.gwt.typedarrays.shared.TypedArrays;


final class DirectReadWriteFloatBufferAdapter extends FloatBuffer implements HasArrayBufferView {

	static FloatBuffer wrap (DirectReadWriteByteBuffer byteBuffer) {
		return new DirectReadWriteFloatBufferAdapter((DirectReadWriteByteBuffer)byteBuffer.slice());
	}

	private final DirectReadWriteByteBuffer byteBuffer;
	private final Float32Array floatArray;

	DirectReadWriteFloatBufferAdapter (DirectReadWriteByteBuffer byteBuffer) {
		super((byteBuffer.capacity() >> 2));
		this.byteBuffer = byteBuffer;
		this.byteBuffer.clear();
		this.floatArray = TypedArrays.createFloat32Array(byteBuffer.byteArray.buffer(), byteBuffer.byteArray.byteOffset(), capacity);
	}

		@Override
	public FloatBuffer asReadOnlyBuffer () {
		DirectReadOnlyFloatBufferAdapter buf = new DirectReadOnlyFloatBufferAdapter(byteBuffer);
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public FloatBuffer compact () {
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
	public FloatBuffer duplicate () {
		DirectReadWriteFloatBufferAdapter buf = new DirectReadWriteFloatBufferAdapter(
			(DirectReadWriteByteBuffer)byteBuffer.duplicate());
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public float get () {
		return floatArray.get(position++);
	}

	@Override
	public float get (int index) {
		return floatArray.get(index);
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
	protected float[] protectedArray () {
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
	public FloatBuffer put (float c) {
		floatArray.set(position++, c);
		return this;
	}

	@Override
	public FloatBuffer put (int index, float c) {
		floatArray.set(index, c);
		return this;
	}

	@Override
	public FloatBuffer slice () {
		byteBuffer.limit(limit << 2);
		byteBuffer.position(position << 2);
		FloatBuffer result = new DirectReadWriteFloatBufferAdapter((DirectReadWriteByteBuffer)byteBuffer.slice());
		byteBuffer.clear();
		return result;
	}

	public ArrayBufferView getTypedArray () {
		return floatArray;
	}

	public int getElementSize () {
		return 4;
	}
}
