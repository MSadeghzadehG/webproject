

package com.badlogic.gdx.graphics.g3d;

import java.util.Comparator;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.Camera;
import com.badlogic.gdx.graphics.Mesh;
import com.badlogic.gdx.graphics.VertexAttributes;
import com.badlogic.gdx.graphics.g3d.model.MeshPart;
import com.badlogic.gdx.graphics.g3d.utils.MeshBuilder;
import com.badlogic.gdx.graphics.g3d.utils.RenderableSorter;
import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.FlushablePool;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.Pool;


public class ModelCache implements Disposable, RenderableProvider {
	
	public interface MeshPool extends Disposable {
		
		Mesh obtain (VertexAttributes vertexAttributes, int vertexCount, int indexCount);

		
		void flush ();
	}

	
	public static class SimpleMeshPool implements MeshPool {
				private Array<Mesh> freeMeshes = new Array<Mesh>();
		private Array<Mesh> usedMeshes = new Array<Mesh>();

		@Override
		public void flush () {
			freeMeshes.addAll(usedMeshes);
			usedMeshes.clear();
		}

		@Override
		public Mesh obtain (VertexAttributes vertexAttributes, int vertexCount, int indexCount) {
			for (int i = 0, n = freeMeshes.size; i < n; ++i) {
				final Mesh mesh = freeMeshes.get(i);
				if (mesh.getVertexAttributes().equals(vertexAttributes) && mesh.getMaxVertices() >= vertexCount
					&& mesh.getMaxIndices() >= indexCount) {
					freeMeshes.removeIndex(i);
					usedMeshes.add(mesh);
					return mesh;
				}
			}
			vertexCount = 1 + (int)Short.MAX_VALUE;
			indexCount = Math.max(1 + (int)Short.MAX_VALUE, 1 << (32 - Integer.numberOfLeadingZeros(indexCount - 1)));
			Mesh result = new Mesh(false, vertexCount, indexCount, vertexAttributes);
			usedMeshes.add(result);
			return result;
		}

		@Override
		public void dispose () {
			for (Mesh m : usedMeshes)
				m.dispose();
			usedMeshes.clear();
			for (Mesh m : freeMeshes)
				m.dispose();
			freeMeshes.clear();
		}
	}

	
	public static class TightMeshPool implements MeshPool {
		private Array<Mesh> freeMeshes = new Array<Mesh>();
		private Array<Mesh> usedMeshes = new Array<Mesh>();

		@Override
		public void flush () {
			freeMeshes.addAll(usedMeshes);
			usedMeshes.clear();
		}

		@Override
		public Mesh obtain (VertexAttributes vertexAttributes, int vertexCount, int indexCount) {
			for (int i = 0, n = freeMeshes.size; i < n; ++i) {
				final Mesh mesh = freeMeshes.get(i);
				if (mesh.getVertexAttributes().equals(vertexAttributes) && mesh.getMaxVertices() == vertexCount
					&& mesh.getMaxIndices() == indexCount) {
					freeMeshes.removeIndex(i);
					usedMeshes.add(mesh);
					return mesh;
				}
			}
			Mesh result = new Mesh(true, vertexCount, indexCount, vertexAttributes);
			usedMeshes.add(result);
			return result;
		}

		@Override
		public void dispose () {
			for (Mesh m : usedMeshes)
				m.dispose();
			usedMeshes.clear();
			for (Mesh m : freeMeshes)
				m.dispose();
			freeMeshes.clear();
		}
	}

	
	public static class Sorter implements RenderableSorter, Comparator<Renderable> {
		@Override
		public void sort (Camera camera, Array<Renderable> renderables) {
			renderables.sort(this);
		}

		@Override
		public int compare (Renderable arg0, Renderable arg1) {
			final VertexAttributes va0 = arg0.meshPart.mesh.getVertexAttributes();
			final VertexAttributes va1 = arg1.meshPart.mesh.getVertexAttributes();
			final int vc = va0.compareTo(va1);
			if (vc == 0) {
				final int mc = arg0.material.compareTo(arg1.material);
				if (mc == 0) {
					return arg0.meshPart.primitiveType - arg1.meshPart.primitiveType;
				}
				return mc;
			}
			return vc;
		}
	}

	private Array<Renderable> renderables = new Array<Renderable>();
	private FlushablePool<Renderable> renderablesPool = new FlushablePool<Renderable>() {
		@Override
		protected Renderable newObject () {
			return new Renderable();
		}
	};
	private FlushablePool<MeshPart> meshPartPool = new FlushablePool<MeshPart>() {
		@Override
		protected MeshPart newObject () {
			return new MeshPart();
		}
	};

	private Array<Renderable> items = new Array<Renderable>();
	private Array<Renderable> tmp = new Array<Renderable>();

	private MeshBuilder meshBuilder;
	private boolean building;
	private RenderableSorter sorter;
	private MeshPool meshPool;
	private Camera camera;

	
	public ModelCache () {
		this(new Sorter(), new SimpleMeshPool());
	}

	
	public ModelCache (RenderableSorter sorter, MeshPool meshPool) {
		this.sorter = sorter;
		this.meshPool = meshPool;
		meshBuilder = new MeshBuilder();
	}

	
	public void begin () {
		begin(null);
	}

	
	public void begin (Camera camera) {
		if (building) throw new GdxRuntimeException("Call end() after calling begin()");
		building = true;

		this.camera = camera;
		renderablesPool.flush();
		renderables.clear();
		items.clear();
		meshPartPool.flush();
		meshPool.flush();
	}

	private Renderable obtainRenderable (Material material, int primitiveType) {
		Renderable result = renderablesPool.obtain();
		result.bones = null;
		result.environment = null;
		result.material = material;
		result.meshPart.mesh = null;
		result.meshPart.offset = 0;
		result.meshPart.size = 0;
		result.meshPart.primitiveType = primitiveType;
		result.meshPart.center.set(0, 0, 0);
		result.meshPart.halfExtents.set(0, 0, 0);
		result.meshPart.radius = -1f;
		result.shader = null;
		result.userData = null;
		result.worldTransform.idt();
		return result;
	}

	
	public void end () {
		if (!building) throw new GdxRuntimeException("Call begin() prior to calling end()");
		building = false;

		if (items.size == 0) return;
		sorter.sort(camera, items);

		int itemCount = items.size;
		int initCount = renderables.size;

		final Renderable first = items.get(0);
		VertexAttributes vertexAttributes = first.meshPart.mesh.getVertexAttributes();
		Material material = first.material;
		int primitiveType = first.meshPart.primitiveType;
		int offset = renderables.size;

		meshBuilder.begin(vertexAttributes);
		MeshPart part = meshBuilder.part("", primitiveType, meshPartPool.obtain());
		renderables.add(obtainRenderable(material, primitiveType));

		for (int i = 0, n = items.size; i < n; ++i) {
			final Renderable renderable = items.get(i);
			final VertexAttributes va = renderable.meshPart.mesh.getVertexAttributes();
			final Material mat = renderable.material;
			final int pt = renderable.meshPart.primitiveType;

			final boolean sameMesh = va.equals(vertexAttributes)
				&& renderable.meshPart.size + meshBuilder.getNumVertices() < Short.MAX_VALUE; 			final boolean samePart = sameMesh && pt == primitiveType && mat.same(material, true);

			if (!samePart) {
				if (!sameMesh) {
					final Mesh mesh = meshBuilder.end(meshPool.obtain(vertexAttributes, meshBuilder.getNumVertices(),
						meshBuilder.getNumIndices()));
					while (offset < renderables.size)
						renderables.get(offset++).meshPart.mesh = mesh;
					meshBuilder.begin(vertexAttributes = va);
				}

				final MeshPart newPart = meshBuilder.part("", pt, meshPartPool.obtain());
				final Renderable previous = renderables.get(renderables.size - 1);
				previous.meshPart.offset = part.offset;
				previous.meshPart.size = part.size;
				part = newPart;

				renderables.add(obtainRenderable(material = mat, primitiveType = pt));
			}

			meshBuilder.setVertexTransform(renderable.worldTransform);
			meshBuilder.addMesh(renderable.meshPart.mesh, renderable.meshPart.offset, renderable.meshPart.size);
		}

		final Mesh mesh = meshBuilder.end(meshPool.obtain(vertexAttributes, meshBuilder.getNumVertices(),
			meshBuilder.getNumIndices()));
		while (offset < renderables.size)
			renderables.get(offset++).meshPart.mesh = mesh;

		final Renderable previous = renderables.get(renderables.size - 1);
		previous.meshPart.offset = part.offset;
		previous.meshPart.size = part.size;
	}

	
	public void add (Renderable renderable) {
		if (!building) throw new GdxRuntimeException("Can only add items to the ModelCache in between .begin() and .end()");
		if (renderable.bones == null)
			items.add(renderable);
		else
			renderables.add(renderable);
	}

	
	public void add (final RenderableProvider renderableProvider) {
		renderableProvider.getRenderables(tmp, renderablesPool);
		for (int i = 0, n = tmp.size; i < n; ++i)
			add(tmp.get(i));
		tmp.clear();
	}

	
	public <T extends RenderableProvider> void add (final Iterable<T> renderableProviders) {
		for (final RenderableProvider renderableProvider : renderableProviders)
			add(renderableProvider);
	}

	@Override
	public void getRenderables (Array<Renderable> renderables, Pool<Renderable> pool) {
		if (building) throw new GdxRuntimeException("Cannot render a ModelCache in between .begin() and .end()");
		for (Renderable r : this.renderables) {
			r.shader = null;
			r.environment = null;
		}
		renderables.addAll(this.renderables);
	}

	@Override
	public void dispose () {
		if (building) throw new GdxRuntimeException("Cannot dispose a ModelCache in between .begin() and .end()");
		meshPool.dispose();
	}
}
