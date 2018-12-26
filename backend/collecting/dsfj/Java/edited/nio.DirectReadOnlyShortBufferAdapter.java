

package java.nio;

import com.google.gwt.typedarrays.shared.ArrayBufferView;
import com.google.gwt.typedarrays.shared.Int16Array;
import com.google.gwt.typedarrays.shared.TypedArrays;


final class DirectReadOnlyShortBufferAdapter extends ShortBuffer implements HasArrayBufferView {

	static ShortBuffer wrap (DirectByteBuffer byteBuffer) {
		return new DirectReadOnlyShortBufferAdapter((DirectByteBuffer)byteBuffer.slice());
	}

	private final DirectByteBuffer byteBuffer;
	private final Int16Array shortArray;

	DirectReadOnlyShortBufferAdapter (DirectByteBuffer byteBuffer) {
		super((byteBuffer.capacity() >> 1));
		this.byteBuffer = byteBuffer;
		this.byteBuffer.clear();
		this.shortArray = TypedArrays.createInt16Array(byteBuffer.byteArray.buffer(), byteBuffer.byteArray.byteOffset(), capacity);
	}

	@Override
	public ShortBuffer asReadOnlyBuffer () {
		DirectReadOnlyShortBufferAdapter buf = new DirectReadOnlyShortBufferAdapter(byteBuffer);
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public ShortBuffer compact () {
		throw new ReadOnlyBufferException();
	}

	@Override
	public ShortBuffer duplicate () {
		DirectReadOnlyShortBufferAdapter buf = new DirectReadOnlyShortBufferAdapter((DirectByteBuffer)byteBuffer.duplicate());
		buf.limit = limit;
		buf.position = position;
		buf.mark = mark;
		return buf;
	}

	@Override
	public short get () {
		return (short)shortArray.get(position++);
	}

	@Override
	public short get (int index) {
		return (short)shortArray.get(index);
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
	protected short[] protectedArray () {
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
	public ShortBuffer put (short c) {
		throw new ReadOnlyBufferException();
	}

	@Override
	public ShortBuffer put (int index, short c) {
		throw new ReadOnlyBufferException();
	}

	@Override
	public ShortBuffer slice () {
		byteBuffer.limit(limit << 1);
		byteBuffer.position(position << 1);
		ShortBuffer result = new DirectReadOnlyShortBufferAdapter((DirectByteBuffer)byteBuffer.slice());
		byteBuffer.clear();
		return result;
	}

	public ArrayBufferView getTypedArray () {
		return shortArray;
	}

	public int getElementSize () {
		return 2;
	}
}
