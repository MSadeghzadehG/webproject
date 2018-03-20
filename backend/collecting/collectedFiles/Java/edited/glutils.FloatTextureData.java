

package com.badlogic.gdx.graphics.glutils;

import java.nio.FloatBuffer;

import com.badlogic.gdx.Application.ApplicationType;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.Graphics;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.GL30;
import com.badlogic.gdx.graphics.Pixmap;
import com.badlogic.gdx.graphics.Pixmap.Format;
import com.badlogic.gdx.graphics.TextureData;
import com.badlogic.gdx.utils.BufferUtils;
import com.badlogic.gdx.utils.GdxRuntimeException;


public class FloatTextureData implements TextureData {

	int width = 0;
	int height = 0;

	int internalFormat;
	int format;
	int type;

	boolean isGpuOnly;

	boolean isPrepared = false;
	FloatBuffer buffer;

	public FloatTextureData (int w, int h, int internalFormat, int format, int type, boolean isGpuOnly) {
		this.width = w;
		this.height = h;
		this.internalFormat = internalFormat;
		this.format = format;
		this.type = type;
		this.isGpuOnly = isGpuOnly;
	}

	@Override
	public TextureDataType getType () {
		return TextureDataType.Custom;
	}

	@Override
	public boolean isPrepared () {
		return isPrepared;
	}

	@Override
	public void prepare () {
		if (isPrepared) throw new GdxRuntimeException("Already prepared");
		if (!isGpuOnly) {
			int amountOfFloats = 4;
			if (Gdx.graphics.getGLVersion().getType().equals(GLVersion.Type.OpenGL)) {
				if (internalFormat == GL30.GL_RGBA16F || internalFormat ==  GL30.GL_RGBA32F) amountOfFloats = 4;
				if (internalFormat == GL30.GL_RGB16F || internalFormat ==  GL30.GL_RGB32F) amountOfFloats = 3;
				if (internalFormat == GL30.GL_RG16F || internalFormat ==  GL30.GL_RG32F) amountOfFloats = 2;
				if (internalFormat == GL30.GL_R16F || internalFormat ==  GL30.GL_R32F) amountOfFloats = 1;
			}
			this.buffer = BufferUtils.newFloatBuffer(width * height * amountOfFloats);
		}
		isPrepared = true;
	}

	@Override
	public void consumeCustomData (int target) {
		if (Gdx.app.getType() == ApplicationType.Android || Gdx.app.getType() == ApplicationType.iOS
			|| Gdx.app.getType() == ApplicationType.WebGL) {

			if (!Gdx.graphics.supportsExtension("OES_texture_float"))
				throw new GdxRuntimeException("Extension OES_texture_float not supported!");

									Gdx.gl.glTexImage2D(target, 0, GL20.GL_RGBA, width, height, 0, GL20.GL_RGBA, GL20.GL_FLOAT, buffer);

		} else {
			if (!Gdx.graphics.isGL30Available()) {
				if (!Gdx.graphics.supportsExtension("GL_ARB_texture_float"))
					throw new GdxRuntimeException("Extension GL_ARB_texture_float not supported!");
			}
									Gdx.gl.glTexImage2D(target, 0, internalFormat, width, height, 0, format, GL20.GL_FLOAT, buffer);
		}
	}

	@Override
	public Pixmap consumePixmap () {
		throw new GdxRuntimeException("This TextureData implementation does not return a Pixmap");
	}

	@Override
	public boolean disposePixmap () {
		throw new GdxRuntimeException("This TextureData implementation does not return a Pixmap");
	}

	@Override
	public int getWidth () {
		return width;
	}

	@Override
	public int getHeight () {
		return height;
	}

	@Override
	public Format getFormat () {
		return Format.RGBA8888; 	}

	@Override
	public boolean useMipMaps () {
		return false;
	}

	@Override
	public boolean isManaged () {
		return true;
	}

	public FloatBuffer getBuffer () {
		return buffer;
	}
}
