

package com.badlogic.gdx.graphics.glutils;

import java.nio.FloatBuffer;

import com.badlogic.gdx.graphics.VertexAttributes;
import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.IntIntMap;


public interface VertexData extends Disposable {
	
	public int getNumVertices ();

	
	public int getNumMaxVertices ();

	
	public VertexAttributes getAttributes ();

	
	public void setVertices (float[] vertices, int offset, int count);

	
	public void updateVertices (int targetOffset, float[] vertices, int sourceOffset, int count);

	
	public FloatBuffer getBuffer ();

	
	public void bind (ShaderProgram shader);

	
	public void bind (ShaderProgram shader, int[] locations);

	
	public void unbind (ShaderProgram shader);

	
	public void unbind (ShaderProgram shader, int[] locations);
	
	
	public void invalidate ();

	
	public void dispose ();
}
