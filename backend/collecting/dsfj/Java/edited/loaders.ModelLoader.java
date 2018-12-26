

package com.badlogic.gdx.assets.loaders;

import java.util.Iterator;

import com.badlogic.gdx.assets.AssetDescriptor;
import com.badlogic.gdx.assets.AssetLoaderParameters;
import com.badlogic.gdx.assets.AssetManager;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g3d.Model;
import com.badlogic.gdx.graphics.g3d.model.data.ModelData;
import com.badlogic.gdx.graphics.g3d.model.data.ModelMaterial;
import com.badlogic.gdx.graphics.g3d.model.data.ModelTexture;
import com.badlogic.gdx.graphics.g3d.utils.TextureProvider;
import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.ObjectMap;

public abstract class ModelLoader<P extends ModelLoader.ModelParameters> extends AsynchronousAssetLoader<Model, P> {
	public ModelLoader (FileHandleResolver resolver) {
		super(resolver);
	}

	protected Array<ObjectMap.Entry<String, ModelData>> items = new Array<ObjectMap.Entry<String, ModelData>>();
	protected ModelParameters defaultParameters = new ModelParameters();

	
	public abstract ModelData loadModelData (final FileHandle fileHandle, P parameters);

	
	public ModelData loadModelData (final FileHandle fileHandle) {
		return loadModelData(fileHandle, null);
	}

	
	public Model loadModel (final FileHandle fileHandle, TextureProvider textureProvider, P parameters) {
		final ModelData data = loadModelData(fileHandle, parameters);
		return data == null ? null : new Model(data, textureProvider);
	}

	
	public Model loadModel (final FileHandle fileHandle, P parameters) {
		return loadModel(fileHandle, new TextureProvider.FileTextureProvider(), parameters);
	}

	
	public Model loadModel (final FileHandle fileHandle, TextureProvider textureProvider) {
		return loadModel(fileHandle, textureProvider, null);
	}

	
	public Model loadModel (final FileHandle fileHandle) {
		return loadModel(fileHandle, new TextureProvider.FileTextureProvider(), null);
	}

	@Override
	public Array<AssetDescriptor> getDependencies (String fileName, FileHandle file, P parameters) {
		final Array<AssetDescriptor> deps = new Array();
		ModelData data = loadModelData(file, parameters);
		if (data == null) return deps;

		ObjectMap.Entry<String, ModelData> item = new ObjectMap.Entry<String, ModelData>();
		item.key = fileName;
		item.value = data;
		synchronized (items) {
			items.add(item);
		}

		TextureLoader.TextureParameter textureParameter = (parameters != null)
				? parameters.textureParameter
				: defaultParameters.textureParameter;

		for (final ModelMaterial modelMaterial : data.materials) {
			if (modelMaterial.textures != null) {
				for (final ModelTexture modelTexture : modelMaterial.textures)
					deps.add(new AssetDescriptor(modelTexture.fileName, Texture.class, textureParameter));
			}
		}
		return deps;
	}

	@Override
	public void loadAsync (AssetManager manager, String fileName, FileHandle file, P parameters) {
	}

	@Override
	public Model loadSync (AssetManager manager, String fileName, FileHandle file, P parameters) {
		ModelData data = null;
		synchronized (items) {
			for (int i = 0; i < items.size; i++) {
				if (items.get(i).key.equals(fileName)) {
					data = items.get(i).value;
					items.removeIndex(i);
				}
			}
		}
		if (data == null) return null;
		final Model result = new Model(data, new TextureProvider.AssetTextureProvider(manager));
						Iterator<Disposable> disposables = result.getManagedDisposables().iterator();
		while (disposables.hasNext()) {
			Disposable disposable = disposables.next();
			if (disposable instanceof Texture) {
				disposables.remove();
			}
		}
		data = null;
		return result;
	}

	static public class ModelParameters extends AssetLoaderParameters<Model> {
		public TextureLoader.TextureParameter textureParameter;

		public ModelParameters() {
			textureParameter = new TextureLoader.TextureParameter();
			textureParameter.minFilter = textureParameter.magFilter = Texture.TextureFilter.Linear;
			textureParameter.wrapU = textureParameter.wrapV = Texture.TextureWrap.Repeat;
		}
	}
}
