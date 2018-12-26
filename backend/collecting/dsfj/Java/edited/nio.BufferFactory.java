

package java.nio;


final class BufferFactory {

	
	public static ByteBuffer newByteBuffer (byte array[]) {
		return new ReadWriteHeapByteBuffer(array);
	}

	
	public static ByteBuffer newByteBuffer (int capacity) {
		return new ReadWriteHeapByteBuffer(capacity);
	}

	
	public static CharBuffer newCharBuffer (char array[]) {
		return new ReadWriteCharArrayBuffer(array);
	}

	
	public static CharBuffer newCharBuffer (CharSequence chseq) {
		return new CharSequenceAdapter(chseq);
	}

	
	public static CharBuffer newCharBuffer (int capacity) {
		return new ReadWriteCharArrayBuffer(capacity);
	}

	
	public static ByteBuffer newDirectByteBuffer (int capacity) {
		return new DirectReadWriteByteBuffer(capacity);
	}

	
	public static DoubleBuffer newDoubleBuffer (double array[]) {
		return new ReadWriteDoubleArrayBuffer(array);
	}

	
	public static DoubleBuffer newDoubleBuffer (int capacity) {
		return new ReadWriteDoubleArrayBuffer(capacity);
	}

	
	public static FloatBuffer newFloatBuffer (float array[]) {
		return new ReadWriteFloatArrayBuffer(array);
	}

	
	public static FloatBuffer newFloatBuffer (int capacity) {
		return new ReadWriteFloatArrayBuffer(capacity);
	}

	
	public static IntBuffer newIntBuffer (int capacity) {
		return new ReadWriteIntArrayBuffer(capacity);
	}

	
	public static IntBuffer newIntBuffer (int array[]) {
		return new ReadWriteIntArrayBuffer(array);
	}

	
	public static LongBuffer newLongBuffer (int capacity) {
		return new ReadWriteLongArrayBuffer(capacity);
	}

	
	public static LongBuffer newLongBuffer (long array[]) {
		return new ReadWriteLongArrayBuffer(array);
	}

	
	public static ShortBuffer newShortBuffer (int capacity) {
		return new ReadWriteShortArrayBuffer(capacity);
	}

	
	public static ShortBuffer newShortBuffer (short array[]) {
		return new ReadWriteShortArrayBuffer(array);
	}

}
