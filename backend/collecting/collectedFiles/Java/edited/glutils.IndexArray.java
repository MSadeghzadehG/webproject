

package com.badlogic.gdx.graphics.glutils;

import java.nio.ByteBuffer;
import java.nio.ShortBuffer;

import com.badlogic.gdx.utils.BufferUtils;

public class IndexArray implements IndexData {
	final ShortBuffer buffer;
	final ByteBuffer byteBuffer;

		private final boolean empty;

	
	public IndexArray (int maxIndices) {

		empty = maxIndices == 0;
		if (empty) {
			maxIndices = 1; 		}

		byteBuffer = BufferUtils.newUnsafeByteBuffer(maxIndices * 2);
		buffer = byteBuffer.asShortBuffer();
		buffer.flip();
		byteBuffer.flip();
	}

	
	public int getNumIndices () {
		return empty ? 0 : buffer.limit();
	}

	
	public int getNumMaxIndices () {
		return empty ? 0 : buffer.capacity();
	}

	
	public void setIndices (short[] indices, int offset, int count) {
		buffer.clear();
		buffer.put(indices, offset, count);
		buffer.flip();
		byteBuffer.position(0);
		byteBuffer.limit(count << 1);
	}
	
	public void setIndices (ShortBuffer indices) {
		int pos = indices.position();
		buffer.clear();
		buffer.limit(indices.remaining());
		buffer.put(indices);
		buffer.flip();
		indices.position(pos);
		byteBuffer.position(0);
		byteBuffer.limit(buffer.limit() << 1);
	}

	@Override
	public void updateIndices (int targetOffset, short[] indices, int offset, int count) {
		final int pos = byteBuffer.position();
		byteBuffer.position(targetOffset * 2);
		BufferUtils.copy(indices, offset, byteBuffer, count);
		byteBuffer.position(pos);
	}

	
	public ShortBuffer getBuffer () {
		return buffer;
	}

	
	public void bind () {
	}

	
	public void unbind () {
	}

	
	public void invalidate () {
	}

	
	public void dispose () {
		BufferUtils.disposeUnsafeByteBuffer(byteBuffer);
	}
}
