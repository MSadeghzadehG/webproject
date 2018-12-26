

package com.badlogic.gdx.graphics.g3d.decals;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.glutils.ShaderProgram;
import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.Sort;


public class SimpleOrthoGroupStrategy implements GroupStrategy {
	private Comparator comparator = new Comparator();
	private static final int GROUP_OPAQUE = 0;
	private static final int GROUP_BLEND = 1;

	@Override
	public int decideGroup (Decal decal) {
		return decal.getMaterial().isOpaque() ? GROUP_OPAQUE : GROUP_BLEND;
	}

	@Override
	public void beforeGroup (int group, Array<Decal> contents) {
		if (group == GROUP_BLEND) {
			Sort.instance().sort(contents, comparator);
			Gdx.gl.glEnable(GL20.GL_BLEND);
									Gdx.gl.glDepthMask(false);
		} else {
					}
	}

	@Override
	public void afterGroup (int group) {
		if (group == GROUP_BLEND) {
			Gdx.gl.glDepthMask(true);
			Gdx.gl.glDisable(GL20.GL_BLEND);
		}
	}

	@Override
	public void beforeGroups () {
		Gdx.gl.glEnable(GL20.GL_TEXTURE_2D);
	}

	@Override
	public void afterGroups () {
		Gdx.gl.glDisable(GL20.GL_TEXTURE_2D);
	}

	class Comparator implements java.util.Comparator<Decal> {
		@Override
		public int compare (Decal a, Decal b) {
			if (a.getZ() == b.getZ()) return 0;
			return a.getZ() - b.getZ() < 0 ? -1 : 1;
		}
	}

	@Override
	public ShaderProgram getGroupShader (int group) {
		return null;
	}
}
