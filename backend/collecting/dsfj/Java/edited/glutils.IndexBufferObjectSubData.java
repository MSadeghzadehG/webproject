

package com.badlogic.gdx.graphics.glutils;

import java.nio.ByteBuffer;
import java.nio.ShortBuffer;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.utils.BufferUtils;
import com.badlogic.gdx.utils.GdxRuntimeException;


public class IndexBufferObjectSubData implements IndexData {
	final ShortBuffer buffer;
	final ByteBuffer byteBuffer;
	int bufferHandle;
	final boolean isDirect;
	boolean isDirty = true;
	boolean isBound = false;
	final int usage;

	
	public IndexBufferObjectSubData (boolean isStatic, int maxIndices) {
		byteBuffer = BufferUtils.newByteBuffer(maxIndices * 2);
		isDirect = true;

		usage = isStatic ? GL20.GL_STATIC_DRAW : GL20.GL_DYNAMIC_DRAW;
		buffer = byteBuffer.asShortBuffer();
		buffer.flip();
		byteBuffer.flip();
		bufferHandle = createBufferObject();
	}

	
	public IndexBufferObjectSubData (int maxIndices) {
		byteBuffer = BufferUtils.newByteBuffer(maxIndices * 2);
		this.isDirect = true;

		usage = GL20.GL_STATIC_DRAW;
		buffer = byteBuffer.asShortBuffer();
		buffer.flip();
		byteBuffer.flip();
		bufferHandle = createBufferObject();
	}

	private int createBufferObject () {
		int result = Gdx.gl20.glGenBuffer();
		Gdx.gl20.glBindBuffer(GL20.GL_ELEMENT_ARRAY_BUFFER, result);
		Gdx.gl20.glBufferData(GL20.GL_ELEMENT_ARRAY_BUFFER, byteBuffer.capacity(), null, usage);
		Gdx.gl20.glBindBuffer(GL20.GL_ELEMENT_ARRAY_BUFFER, 0);
		return result;
	}

	
	public int getNumIndices () {
		return buffer.limit();
	}

	
	public int getNumMaxIndices () {
		return buffer.capacity();
	}

	
	public void setIndices (short[] indices, int offset, int count) {
		isDirty = true;
		buffer.clear();
		buffer.put(indices, offset, count);
		buffer.flip();
		byteBuffer.position(0);
		byteBuffer.limit(count << 1);

		if (isBound) {
			Gdx.gl20.glBufferSubData(GL20.GL_ELEMENT_ARRAY_BUFFER, 0, byteBuffer.limit(), byteBuffer);
			isDirty = false;
		}
	}

	public void setIndices (ShortBuffer indices) {
		int pos = indices.position();
		isDirty = true;
		buffer.clear();
		buffer.put(indices);
		buffer.flip();
		indices.position(pos);
		byteBuffer.position(0);
		byteBuffer.limit(buffer.limit() << 1);
		
		if (isBound) {
			Gdx.gl20.glBufferSubData(GL20.GL_ELEMENT_ARRAY_BUFFER, 0, byteBuffer.limit(), byteBuffer);
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
			Gdx.gl20.glBufferSubData(GL20.GL_ELEMENT_ARRAY_BUFFER, 0, byteBuffer.limit(), byteBuffer);
			isDirty = false;
		}
	}

	
	
	public ShortBuffer getBuffer () {
		isDirty = true;
		return buffer;
	}

	
	public void bind () {
		if (bufferHandle == 0) throw new GdxRuntimeException("IndexBufferObject cannot be used after it has been disposed.");

		Gdx.gl20.glBindBuffer(GL20.GL_ELEMENT_ARRAY_BUFFER, bufferHandle);
		if (isDirty) {
			byteBuffer.limit(buffer.limit() * 2);
			Gdx.gl20.glBufferSubData(GL20.GL_ELEMENT_ARRAY_BUFFER, 0, byteBuffer.limit(), byteBuffer);
			isDirty = false;
		}
		isBound = true;
	}

	
	public void unbind () {
		Gdx.gl20.glBindBuffer(GL20.GL_ELEMENT_ARRAY_BUFFER, 0);
		isBound = false;
	}

	
	public void invalidate () {
		bufferHandle = createBufferObject();
		isDirty = true;
	}

	
	public void dispose () {
		GL20 gl = Gdx.gl20;
		gl.glBindBuffer(GL20.GL_ELEMENT_ARRAY_BUFFER, 0);
		gl.glDeleteBuffer(bufferHandle);
		bufferHandle = 0;
	}
}
