

package com.badlogic.gdx.graphics.glutils;

import java.nio.ByteBuffer;
import java.nio.ShortBuffer;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.utils.BufferUtils;
import com.badlogic.gdx.utils.GdxRuntimeException;


public class IndexBufferObject implements IndexData {
	final ShortBuffer buffer;
	final ByteBuffer byteBuffer;
	int bufferHandle;
	final boolean isDirect;
	boolean isDirty = true;
	boolean isBound = false;
	final int usage;

		private final boolean empty;

	
	public IndexBufferObject (int maxIndices) {
		this(true, maxIndices);
	}

	
	public IndexBufferObject (boolean isStatic, int maxIndices) {

		empty = maxIndices == 0;
		if (empty) {
			maxIndices = 1; 		}

		byteBuffer = BufferUtils.newUnsafeByteBuffer(maxIndices * 2);
		isDirect = true;

		buffer = byteBuffer.asShortBuffer();
		buffer.flip();
		byteBuffer.flip();
		bufferHandle = Gdx.gl20.glGenBuffer();
		usage = isStatic ? GL20.GL_STATIC_DRAW : GL20.GL_DYNAMIC_DRAW;
	}

	
	public int getNumIndices () {
		return empty ? 0 : buffer.limit();
	}

	
	public int getNumMaxIndices () {
		return empty ? 0 : buffer.capacity();
	}

	
	public void setIndices (short[] indices, int offset, int count) {
		isDirty = true;
		buffer.clear();
		buffer.put(indices, offset, count);
		buffer.flip();
		byteBuffer.position(0);
		byteBuffer.limit(count << 1);

		if (isBound) {
			Gdx.gl20.glBufferData(GL20.GL_ELEMENT_ARRAY_BUFFER, byteBuffer.limit(), byteBuffer, usage);
			isDirty = false;
		}
	}

	public void setIndices (ShortBuffer indices) {
		isDirty = true;
		int pos = indices.position();
		buffer.clear();
		buffer.put(indices);
		buffer.flip();
		indices.position(pos);
		byteBuffer.position(0);
		byteBuffer.limit(buffer.limit() << 1);

		if (isBound) {
			Gdx.gl20.glBufferData(GL20.GL_ELEMENT_ARRAY_BUFFER, byteBuffer.limit(), byteBuffer, usage);
			isDirty = false;
		}
	}

	@Override
	public void updateIndices (int targetOffset, short[] indices, int offset, int count) {
		isDirty = true;
		final int pos = byteBuffer.position();
		byteBuffer.position(targetOffset * 2);
		BufferUtils.copy(indices, offset, byteBuffer, count);
		byteBuffer.position(pos);
		buffer.position(0);

		if (isBound) {
			Gdx.gl20.glBufferData(GL20.GL_ELEMENT_ARRAY_BUFFER, byteBuffer.limit(), byteBuffer, usage);
			isDirty = false;
		}
	}

	
	public ShortBuffer getBuffer () {
		isDirty = true;
		return buffer;
	}

	
	public void bind () {
		if (bufferHandle == 0) throw new GdxRuntimeException("No buffer allocated!");

		Gdx.gl20.glBindBuffer(GL20.GL_ELEMENT_ARRAY_BUFFER, bufferHandle);
		if (isDirty) {
			byteBuffer.limit(buffer.limit() * 2);
			Gdx.gl20.glBufferData(GL20.GL_ELEMENT_ARRAY_BUFFER, byteBuffer.limit(), byteBuffer, usage);
			isDirty = false;
		}
		isBound = true;
	}

	
	public void unbind () {
		Gdx.gl20.glBindBuffer(GL20.GL_ELEMENT_ARRAY_BUFFER, 0);
		isBound = false;
	}

	
	public void invalidate () {
		bufferHandle = Gdx.gl20.glGenBuffer();
		isDirty = true;
	}

	
	public void dispose () {
		Gdx.gl20.glBindBuffer(GL20.GL_ELEMENT_ARRAY_BUFFER, 0);
		Gdx.gl20.glDeleteBuffer(bufferHandle);
		bufferHandle = 0;

		BufferUtils.disposeUnsafeByteBuffer(byteBuffer);
	}
}
