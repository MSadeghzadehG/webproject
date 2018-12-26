

package com.badlogic.gdx.graphics.g3d.decals;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.Mesh;
import com.badlogic.gdx.graphics.VertexAttribute;
import com.badlogic.gdx.graphics.VertexAttributes;
import com.badlogic.gdx.graphics.glutils.ShaderProgram;
import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.Pool;
import com.badlogic.gdx.utils.SortedIntList;


public class DecalBatch implements Disposable {
	private static final int DEFAULT_SIZE = 1000;
	private float[] vertices;
	private Mesh mesh;

	private final SortedIntList<Array<Decal>> groupList = new SortedIntList<Array<Decal>>();
	private GroupStrategy groupStrategy;
	private final Pool<Array<Decal>> groupPool = new Pool<Array<Decal>>(16) {
		@Override
		protected Array<Decal> newObject () {
			return new Array<Decal>(false, 100);
		}
	};
	private final Array<Array<Decal>> usedGroups = new Array<Array<Decal>>(16);

	
	public DecalBatch (GroupStrategy groupStrategy) {
		this(DEFAULT_SIZE, groupStrategy);
	}

	public DecalBatch (int size, GroupStrategy groupStrategy) {
		initialize(size);
		setGroupStrategy(groupStrategy);
	}

	
	public void setGroupStrategy (GroupStrategy groupStrategy) {
		this.groupStrategy = groupStrategy;
	}

	
	public void initialize (int size) {
		vertices = new float[size * Decal.SIZE];

		Mesh.VertexDataType vertexDataType = Mesh.VertexDataType.VertexArray;
		if(Gdx.gl30 != null) {
			vertexDataType = Mesh.VertexDataType.VertexBufferObjectWithVAO;
		}
		mesh = new Mesh(vertexDataType, false, size * 4, size * 6, new VertexAttribute(
				VertexAttributes.Usage.Position, 3, ShaderProgram.POSITION_ATTRIBUTE), new VertexAttribute(
				VertexAttributes.Usage.ColorPacked, 4, ShaderProgram.COLOR_ATTRIBUTE), new VertexAttribute(
				VertexAttributes.Usage.TextureCoordinates, 2, ShaderProgram.TEXCOORD_ATTRIBUTE + "0"));

		short[] indices = new short[size * 6];
		int v = 0;
		for (int i = 0; i < indices.length; i += 6, v += 4) {
			indices[i] = (short)(v);
			indices[i + 1] = (short)(v + 2);
			indices[i + 2] = (short)(v + 1);
			indices[i + 3] = (short)(v + 1);
			indices[i + 4] = (short)(v + 2);
			indices[i + 5] = (short)(v + 3);
		}
		mesh.setIndices(indices);
	}

	
	public int getSize () {
		return vertices.length / Decal.SIZE;
	}

	
	public void add (Decal decal) {
		int groupIndex = groupStrategy.decideGroup(decal);
		Array<Decal> targetGroup = groupList.get(groupIndex);
		if (targetGroup == null) {
			targetGroup = groupPool.obtain();
			targetGroup.clear();
			usedGroups.add(targetGroup);
			groupList.insert(groupIndex, targetGroup);
		}
		targetGroup.add(decal);
	}

	
	public void flush () {
		render();
		clear();
	}

	
	protected void render () {
		groupStrategy.beforeGroups();
		for (SortedIntList.Node<Array<Decal>> group : groupList) {
			groupStrategy.beforeGroup(group.index, group.value);
			ShaderProgram shader = groupStrategy.getGroupShader(group.index);
			render(shader, group.value);
			groupStrategy.afterGroup(group.index);
		}
		groupStrategy.afterGroups();
	}

	
	private void render (ShaderProgram shader, Array<Decal> decals) {
				DecalMaterial lastMaterial = null;
		int idx = 0;
		for (Decal decal : decals) {
			if (lastMaterial == null || !lastMaterial.equals(decal.getMaterial())) {
				if (idx > 0) {
					flush(shader, idx);
					idx = 0;
				}
				decal.material.set();
				lastMaterial = decal.material;
			}
			decal.update();
			System.arraycopy(decal.vertices, 0, vertices, idx, decal.vertices.length);
			idx += decal.vertices.length;
						if (idx == vertices.length) {
				flush(shader, idx);
				idx = 0;
			}
		}
				if (idx > 0) {
			flush(shader, idx);
		}
	}

	
	protected void flush (ShaderProgram shader, int verticesPosition) {
		mesh.setVertices(vertices, 0, verticesPosition);
		mesh.render(shader, GL20.GL_TRIANGLES, 0, verticesPosition / 4);
	}

	
	protected void clear () {
		groupList.clear();
		groupPool.freeAll(usedGroups);
		usedGroups.clear();
	}

	
	public void dispose () {
		clear();
		vertices = null;
		mesh.dispose();
	}
}
