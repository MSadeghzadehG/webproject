

package java.nio;

import com.google.gwt.typedarrays.shared.ArrayBufferView;
import com.google.gwt.typedarrays.shared.Float32Array;
import com.google.gwt.typedarrays.shared.TypedArrays;


final class DirectReadOnlyFloatBufferAdapter extends FloatBuffer implements HasArrayBufferView {

	static FloatBuffer wrap (DirectByteBuffer byteBuffer) {
		return new DirectReadOnlyFloatBufferAdapter((DirectByteBuffer)byteBuffer.slice());
	}

	private final DirectByteBuffer byteBuffer;
	private final Float32Array floatArray;

	DirectReadOnlyFloatBufferAdapter (DirectByteBuffer byteBuffer) {
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
		throw new ReadOnlyBufferException();
	}

	@Override
	public FloatBuffer duplicate () {
		DirectReadOnlyFloatBufferAdapter buf = new DirectReadOnlyFloatBufferAdapter((DirectByteBuffer)byteBuffer.duplicate());
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
		return true;
	}

	@Override
	public ByteOrder order () {
		return byteBuffer.order();
	}

	@Override
	float[] protectedArray () {
		throw new UnsupportedOperationException();
	}

	@Override
	int protectedArrayOffset () {
		throw new UnsupportedOperationException();
	}

	@Override
	boolean protectedHasArray () {
		return false;
	}

	@Override
	public FloatBuffer put (float c) {
		throw new ReadOnlyBufferException();
	}

	@Override
	public FloatBuffer put (int index, float c) {
		throw new ReadOnlyBufferException();
	}

	@Override
	public FloatBuffer slice () {
		byteBuffer.limit(limit << 2);
		byteBuffer.position(position << 2);
		FloatBuffer result = new DirectReadOnlyFloatBufferAdapter((DirectByteBuffer)byteBuffer.slice());
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
