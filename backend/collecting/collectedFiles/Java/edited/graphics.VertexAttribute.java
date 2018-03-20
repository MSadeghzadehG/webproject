

package com.badlogic.gdx.graphics;

import java.nio.FloatBuffer;


import com.badlogic.gdx.graphics.VertexAttributes.Usage;
import com.badlogic.gdx.graphics.glutils.ShaderProgram;
import com.badlogic.gdx.graphics.glutils.VertexArray;


public final class VertexAttribute {
	
	public final int usage;
	
	public final int numComponents;
	
	public final boolean normalized;
	
	public final int type;
	
	public int offset;
	
	public String alias;
	
	public int unit;
	private final int usageIndex;

	
	public VertexAttribute (int usage, int numComponents, String alias) {
		this(usage, numComponents, alias, 0);
	}

	
	public VertexAttribute (int usage, int numComponents, String alias, int unit) {
		this(usage, numComponents, usage == Usage.ColorPacked ? GL20.GL_UNSIGNED_BYTE : GL20.GL_FLOAT, 
				usage == Usage.ColorPacked, alias, unit);
	}

	
	public VertexAttribute (int usage, int numComponents, int type, boolean normalized, String alias) {
		this(usage, numComponents, type, normalized, alias, 0);
	}
	
	
	public VertexAttribute (int usage, int numComponents, int type, boolean normalized, String alias, int unit) {
		this.usage = usage;
		this.numComponents = numComponents;
		this.type = type;
		this.normalized = normalized;
		this.alias = alias;
		this.unit = unit;
		this.usageIndex = Integer.numberOfTrailingZeros(usage);
	}
	
	
	public VertexAttribute copy (){
		return new VertexAttribute(usage, numComponents, type, normalized, alias, unit);
	}

	public static VertexAttribute Position () {
		return new VertexAttribute(Usage.Position, 3, ShaderProgram.POSITION_ATTRIBUTE);
	}

	public static VertexAttribute TexCoords (int unit) {
		return new VertexAttribute(Usage.TextureCoordinates, 2, ShaderProgram.TEXCOORD_ATTRIBUTE + unit, unit);
	}

	public static VertexAttribute Normal () {
		return new VertexAttribute(Usage.Normal, 3, ShaderProgram.NORMAL_ATTRIBUTE);
	}
	
	public static VertexAttribute ColorPacked () {
		return new VertexAttribute(Usage.ColorPacked, 4, GL20.GL_UNSIGNED_BYTE, true, ShaderProgram.COLOR_ATTRIBUTE);
	}

	public static VertexAttribute ColorUnpacked () {
		return new VertexAttribute(Usage.ColorUnpacked, 4, GL20.GL_FLOAT, false, ShaderProgram.COLOR_ATTRIBUTE);
	}

	public static VertexAttribute Tangent () {
		return new VertexAttribute(Usage.Tangent, 3, ShaderProgram.TANGENT_ATTRIBUTE);
	}

	public static VertexAttribute Binormal () {
		return new VertexAttribute(Usage.BiNormal, 3, ShaderProgram.BINORMAL_ATTRIBUTE);
	}

	public static VertexAttribute BoneWeight (int unit) {
		return new VertexAttribute(Usage.BoneWeight, 2, ShaderProgram.BONEWEIGHT_ATTRIBUTE + unit, unit);
	}

	
	@Override
	public boolean equals (final Object obj) {
		if (!(obj instanceof VertexAttribute)) {
			return false;
		}
		return equals((VertexAttribute)obj);
	}

	public boolean equals (final VertexAttribute other) {
		return other != null && usage == other.usage && numComponents == other.numComponents 
			&& type == other.type && normalized == other.normalized && alias.equals(other.alias)
			&& unit == other.unit;
	}

	
	public int getKey () {
		return (usageIndex << 8) + (unit & 0xFF);
	}
	
	
	public int getSizeInBytes () {
		switch (type) {
		case GL20.GL_FLOAT:
		case GL20.GL_FIXED:
			return 4 * numComponents;
		case GL20.GL_UNSIGNED_BYTE:
		case GL20.GL_BYTE:
			return numComponents;
		case GL20.GL_UNSIGNED_SHORT:
		case GL20.GL_SHORT:
			return 2 * numComponents;
		}
		return 0;
	}

	@Override
	public int hashCode () {
		int result = getKey();
		result = 541 * result + numComponents;
		result = 541 * result + alias.hashCode();
		return result;
	}
}
