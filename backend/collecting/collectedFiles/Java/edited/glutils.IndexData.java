

package com.badlogic.gdx.graphics.glutils;

import java.nio.ShortBuffer;

import com.badlogic.gdx.utils.Disposable;


public interface IndexData extends Disposable {
	
	public int getNumIndices ();

	
	public int getNumMaxIndices ();

	
	public void setIndices (short[] indices, int offset, int count);

	
	public void setIndices (ShortBuffer indices);

	
	public void updateIndices (int targetOffset, short[] indices, int offset, int count);

	
	public ShortBuffer getBuffer ();

	
	public void bind ();

	
	public void unbind ();

	
	public void invalidate ();

	
	public void dispose ();
}
