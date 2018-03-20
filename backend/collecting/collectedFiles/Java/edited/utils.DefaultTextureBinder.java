

package com.badlogic.gdx.graphics.g3d.utils;

import java.nio.IntBuffer;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.GLTexture;
import com.badlogic.gdx.utils.BufferUtils;
import com.badlogic.gdx.utils.GdxRuntimeException;


public final class DefaultTextureBinder implements TextureBinder {
	public final static int ROUNDROBIN = 0;
	public final static int WEIGHTED = 1;
	
	public final static int MAX_GLES_UNITS = 32;
	
	private final int offset;
	
	private final int count;
	
	private final int reuseWeight;
	
	private final GLTexture[] textures;
	
	private final int[] weights;
	
	private final int method;
	
	private boolean reused;

	private int reuseCount = 0; 	private int bindCount = 0; 
	
	public DefaultTextureBinder (final int method) {
		this(method, 0);
	}

	
	public DefaultTextureBinder (final int method, final int offset) {
		this(method, offset, -1);
	}

	
	public DefaultTextureBinder (final int method, final int offset, final int count) {
		this(method, offset, count, 10);
	}

	public DefaultTextureBinder (final int method, final int offset, int count, final int reuseWeight) {
		final int max = Math.min(getMaxTextureUnits(), MAX_GLES_UNITS);
		if (count < 0) count = max - offset;
		if (offset < 0 || count < 0 || (offset + count) > max || reuseWeight < 1)
			throw new GdxRuntimeException("Illegal arguments");
		this.method = method;
		this.offset = offset;
		this.count = count;
		this.textures = new GLTexture[count];
		this.reuseWeight = reuseWeight;
		this.weights = (method == WEIGHTED) ? new int[count] : null;
	}

	private static int getMaxTextureUnits () {
		IntBuffer buffer = BufferUtils.newIntBuffer(16);
		Gdx.gl.glGetIntegerv(GL20.GL_MAX_TEXTURE_IMAGE_UNITS, buffer);
		return buffer.get(0);
	}

	@Override
	public void begin () {
		for (int i = 0; i < count; i++) {
			textures[i] = null;
			if (weights != null) weights[i] = 0;
		}
	}

	@Override
	public void end () {
		
		Gdx.gl.glActiveTexture(GL20.GL_TEXTURE0);
	}

	@Override
	public final int bind (final TextureDescriptor textureDesc) {
		return bindTexture(textureDesc, false);
	}

	private final TextureDescriptor tempDesc = new TextureDescriptor();

	@Override
	public final int bind (final GLTexture texture) {
		tempDesc.set(texture, null, null, null, null);
		return bindTexture(tempDesc, false);
	}

	private final int bindTexture (final TextureDescriptor textureDesc, final boolean rebind) {
		final int idx, result;
		final GLTexture texture = textureDesc.texture;
		reused = false;

		switch (method) {
		case ROUNDROBIN:
			result = offset + (idx = bindTextureRoundRobin(texture));
			break;
		case WEIGHTED:
			result = offset + (idx = bindTextureWeighted(texture));
			break;
		default:
			return -1;
		}

		if (reused) {
			reuseCount++;
			if (rebind)
				texture.bind(result);
			else
				Gdx.gl.glActiveTexture(GL20.GL_TEXTURE0 + result);
		} else
			bindCount++;
		texture.unsafeSetWrap(textureDesc.uWrap, textureDesc.vWrap);
		texture.unsafeSetFilter(textureDesc.minFilter, textureDesc.magFilter);
		return result;
	}

	private int currentTexture = 0;

	private final int bindTextureRoundRobin (final GLTexture texture) {
		for (int i = 0; i < count; i++) {
			final int idx = (currentTexture + i) % count;
			if (textures[idx] == texture) {
				reused = true;
				return idx;
			}
		}
		currentTexture = (currentTexture + 1) % count;
		textures[currentTexture] = texture;
		texture.bind(offset + currentTexture);
		return currentTexture;
	}

	private final int bindTextureWeighted (final GLTexture texture) {
		int result = -1;
		int weight = weights[0];
		int windex = 0;
		for (int i = 0; i < count; i++) {
			if (textures[i] == texture) {
				result = i;
				weights[i] += reuseWeight;
			} else if (weights[i] < 0 || --weights[i] < weight) {
				weight = weights[i];
				windex = i;
			}
		}
		if (result < 0) {
			textures[windex] = texture;
			weights[windex] = 100;
			texture.bind(offset + (result = windex));
		} else
			reused = true;
		return result;
	}

	@Override
	public final int getBindCount () {
		return bindCount;
	}

	@Override
	public final int getReuseCount () {
		return reuseCount;
	}

	@Override
	public final void resetCounts () {
		bindCount = reuseCount = 0;
	}
}
