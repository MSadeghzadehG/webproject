

package com.badlogic.gdx.assets;

import java.util.Stack;

import com.badlogic.gdx.Application;
import com.badlogic.gdx.assets.loaders.AssetLoader;
import com.badlogic.gdx.assets.loaders.BitmapFontLoader;
import com.badlogic.gdx.assets.loaders.CubemapLoader;
import com.badlogic.gdx.assets.loaders.FileHandleResolver;
import com.badlogic.gdx.assets.loaders.I18NBundleLoader;
import com.badlogic.gdx.assets.loaders.MusicLoader;
import com.badlogic.gdx.assets.loaders.ParticleEffectLoader;
import com.badlogic.gdx.assets.loaders.PixmapLoader;
import com.badlogic.gdx.assets.loaders.ShaderProgramLoader;
import com.badlogic.gdx.assets.loaders.SkinLoader;
import com.badlogic.gdx.assets.loaders.SoundLoader;
import com.badlogic.gdx.assets.loaders.TextureAtlasLoader;
import com.badlogic.gdx.assets.loaders.TextureLoader;
import com.badlogic.gdx.assets.loaders.resolvers.InternalFileHandleResolver;
import com.badlogic.gdx.audio.Music;
import com.badlogic.gdx.audio.Sound;
import com.badlogic.gdx.graphics.Cubemap;
import com.badlogic.gdx.graphics.Pixmap;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.BitmapFont;
import com.badlogic.gdx.graphics.g2d.ParticleEffect;
import com.badlogic.gdx.graphics.g2d.PolygonRegion;
import com.badlogic.gdx.graphics.g2d.PolygonRegionLoader;
import com.badlogic.gdx.graphics.g2d.TextureAtlas;
import com.badlogic.gdx.graphics.g3d.Model;
import com.badlogic.gdx.graphics.g3d.loader.G3dModelLoader;
import com.badlogic.gdx.graphics.g3d.loader.ObjLoader;
import com.badlogic.gdx.graphics.glutils.ShaderProgram;
import com.badlogic.gdx.scenes.scene2d.ui.Skin;
import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.I18NBundle;
import com.badlogic.gdx.utils.JsonReader;
import com.badlogic.gdx.utils.Logger;
import com.badlogic.gdx.utils.ObjectIntMap;
import com.badlogic.gdx.utils.ObjectMap;
import com.badlogic.gdx.utils.ObjectSet;
import com.badlogic.gdx.utils.TimeUtils;
import com.badlogic.gdx.utils.UBJsonReader;
import com.badlogic.gdx.utils.async.AsyncExecutor;
import com.badlogic.gdx.utils.async.ThreadUtils;
import com.badlogic.gdx.utils.reflect.ClassReflection;


public class AssetManager implements Disposable {
	final ObjectMap<Class, ObjectMap<String, RefCountedContainer>> assets = new ObjectMap();
	final ObjectMap<String, Class> assetTypes = new ObjectMap();
	final ObjectMap<String, Array<String>> assetDependencies = new ObjectMap();
	final ObjectSet<String> injected = new ObjectSet();

	final ObjectMap<Class, ObjectMap<String, AssetLoader>> loaders = new ObjectMap();
	final Array<AssetDescriptor> loadQueue = new Array();
	final AsyncExecutor executor;

	final Stack<AssetLoadingTask> tasks = new Stack();
	AssetErrorListener listener = null;
	int loaded = 0;
	int toLoad = 0;
	int peakTasks = 0;
        
	final FileHandleResolver resolver;

	Logger log = new Logger("AssetManager", Application.LOG_NONE);

	
	public AssetManager () {
		this(new InternalFileHandleResolver());
	}

	
	public AssetManager (FileHandleResolver resolver) {
		this(resolver, true);
	}

	
	public AssetManager (FileHandleResolver resolver, boolean defaultLoaders) {
		this.resolver = resolver;
		if (defaultLoaders) {
			setLoader(BitmapFont.class, new BitmapFontLoader(resolver));
			setLoader(Music.class, new MusicLoader(resolver));
			setLoader(Pixmap.class, new PixmapLoader(resolver));
			setLoader(Sound.class, new SoundLoader(resolver));
			setLoader(TextureAtlas.class, new TextureAtlasLoader(resolver));
			setLoader(Texture.class, new TextureLoader(resolver));
			setLoader(Skin.class, new SkinLoader(resolver));
			setLoader(ParticleEffect.class, new ParticleEffectLoader(resolver));
			setLoader(com.badlogic.gdx.graphics.g3d.particles.ParticleEffect.class,
				new com.badlogic.gdx.graphics.g3d.particles.ParticleEffectLoader(resolver));
			setLoader(PolygonRegion.class, new PolygonRegionLoader(resolver));
			setLoader(I18NBundle.class, new I18NBundleLoader(resolver));
			setLoader(Model.class, ".g3dj", new G3dModelLoader(new JsonReader(), resolver));
			setLoader(Model.class, ".g3db", new G3dModelLoader(new UBJsonReader(), resolver));
			setLoader(Model.class, ".obj", new ObjLoader(resolver));
			setLoader(ShaderProgram.class, new ShaderProgramLoader(resolver));
			setLoader(Cubemap.class, new CubemapLoader(resolver));
		}
		executor = new AsyncExecutor(1);
	}

	
	public FileHandleResolver getFileHandleResolver () {
		return resolver;
	}

	
	public synchronized <T> T get (String fileName) {
		Class<T> type = assetTypes.get(fileName);
		if (type == null) throw new GdxRuntimeException("Asset not loaded: " + fileName);
		ObjectMap<String, RefCountedContainer> assetsByType = assets.get(type);
		if (assetsByType == null) throw new GdxRuntimeException("Asset not loaded: " + fileName);
		RefCountedContainer assetContainer = assetsByType.get(fileName);
		if (assetContainer == null) throw new GdxRuntimeException("Asset not loaded: " + fileName);
		T asset = assetContainer.getObject(type);
		if (asset == null) throw new GdxRuntimeException("Asset not loaded: " + fileName);
		return asset;
	}

	
	public synchronized <T> T get (String fileName, Class<T> type) {
		ObjectMap<String, RefCountedContainer> assetsByType = assets.get(type);
		if (assetsByType == null) throw new GdxRuntimeException("Asset not loaded: " + fileName);
		RefCountedContainer assetContainer = assetsByType.get(fileName);
		if (assetContainer == null) throw new GdxRuntimeException("Asset not loaded: " + fileName);
		T asset = assetContainer.getObject(type);
		if (asset == null) throw new GdxRuntimeException("Asset not loaded: " + fileName);
		return asset;
	}

	
	public synchronized <T> Array<T> getAll (Class<T> type, Array<T> out) {
		ObjectMap<String, RefCountedContainer> assetsByType = assets.get(type);
		if (assetsByType != null) {
			for (ObjectMap.Entry<String, RefCountedContainer> asset : assetsByType.entries()) {
				out.add(asset.value.getObject(type));
			}
		}
		return out;
	}

	
	public synchronized <T> T get (AssetDescriptor<T> assetDescriptor) {
		return get(assetDescriptor.fileName, assetDescriptor.type);
	}

	
	public synchronized void unload (String fileName) {
						if (tasks.size() > 0) {
			AssetLoadingTask currAsset = tasks.firstElement();
			if (currAsset.assetDesc.fileName.equals(fileName)) {
				currAsset.cancel = true;
				log.debug("Unload (from tasks): " + fileName);
				return;
			}
		}

				int foundIndex = -1;
		for (int i = 0; i < loadQueue.size; i++) {
			if (loadQueue.get(i).fileName.equals(fileName)) {
				foundIndex = i;
				break;
			}
		}
		if (foundIndex != -1) {
			toLoad--;
			loadQueue.removeIndex(foundIndex);
			log.debug("Unload (from queue): " + fileName);
			return;
		}

				Class type = assetTypes.get(fileName);
		if (type == null) throw new GdxRuntimeException("Asset not loaded: " + fileName);

		RefCountedContainer assetRef = assets.get(type).get(fileName);

				assetRef.decRefCount();
		if (assetRef.getRefCount() <= 0) {
			log.debug("Unload (dispose): " + fileName);

						if (assetRef.getObject(Object.class) instanceof Disposable) ((Disposable)assetRef.getObject(Object.class)).dispose();

						assetTypes.remove(fileName);
			assets.get(type).remove(fileName);
		} else {
			log.debug("Unload (decrement): " + fileName);
		}

				Array<String> dependencies = assetDependencies.get(fileName);
		if (dependencies != null) {
			for (String dependency : dependencies) {
				if (isLoaded(dependency)) unload(dependency);
			}
		}
				if (assetRef.getRefCount() <= 0) {
			assetDependencies.remove(fileName);
		}
	}

	
	public synchronized <T> boolean containsAsset (T asset) {
		ObjectMap<String, RefCountedContainer> typedAssets = assets.get(asset.getClass());
		if (typedAssets == null) return false;
		for (String fileName : typedAssets.keys()) {
			T otherAsset = (T)typedAssets.get(fileName).getObject(Object.class);
			if (otherAsset == asset || asset.equals(otherAsset)) return true;
		}
		return false;
	}

	
	public synchronized <T> String getAssetFileName (T asset) {
		for (Class assetType : assets.keys()) {
			ObjectMap<String, RefCountedContainer> typedAssets = assets.get(assetType);
			for (String fileName : typedAssets.keys()) {
				T otherAsset = (T)typedAssets.get(fileName).getObject(Object.class);
				if (otherAsset == asset || asset.equals(otherAsset)) return fileName;
			}
		}
		return null;
	}

	
	public synchronized boolean isLoaded (String fileName) {
		if (fileName == null) return false;
		return assetTypes.containsKey(fileName);
	}

	
	public synchronized boolean isLoaded (String fileName, Class type) {
		ObjectMap<String, RefCountedContainer> assetsByType = assets.get(type);
		if (assetsByType == null) return false;
		RefCountedContainer assetContainer = assetsByType.get(fileName);
		if (assetContainer == null) return false;
		return assetContainer.getObject(type) != null;
	}

	
	public <T> AssetLoader getLoader (final Class<T> type) {
		return getLoader(type, null);
	}

	
	public <T> AssetLoader getLoader (final Class<T> type, final String fileName) {
		final ObjectMap<String, AssetLoader> loaders = this.loaders.get(type);
		if (loaders == null || loaders.size < 1) return null;
		if (fileName == null) return loaders.get("");
		AssetLoader result = null;
		int l = -1;
		for (ObjectMap.Entry<String, AssetLoader> entry : loaders.entries()) {
			if (entry.key.length() > l && fileName.endsWith(entry.key)) {
				result = entry.value;
				l = entry.key.length();
			}
		}
		return result;
	}

	
	public synchronized <T> void load (String fileName, Class<T> type) {
		load(fileName, type, null);
	}

	
	public synchronized <T> void load (String fileName, Class<T> type, AssetLoaderParameters<T> parameter) {
		AssetLoader loader = getLoader(type, fileName);
		if (loader == null) throw new GdxRuntimeException("No loader for type: " + ClassReflection.getSimpleName(type));

				if (loadQueue.size == 0) {
			loaded = 0;
			toLoad = 0;
			peakTasks = 0;
		}

		
				for (int i = 0; i < loadQueue.size; i++) {
			AssetDescriptor desc = loadQueue.get(i);
			if (desc.fileName.equals(fileName) && !desc.type.equals(type))
				throw new GdxRuntimeException("Asset with name '" + fileName
					+ "' already in preload queue, but has different type (expected: " + ClassReflection.getSimpleName(type)
					+ ", found: " + ClassReflection.getSimpleName(desc.type) + ")");
		}

				for (int i = 0; i < tasks.size(); i++) {
			AssetDescriptor desc = tasks.get(i).assetDesc;
			if (desc.fileName.equals(fileName) && !desc.type.equals(type))
				throw new GdxRuntimeException("Asset with name '" + fileName
					+ "' already in task list, but has different type (expected: " + ClassReflection.getSimpleName(type) + ", found: "
					+ ClassReflection.getSimpleName(desc.type) + ")");
		}

				Class otherType = assetTypes.get(fileName);
		if (otherType != null && !otherType.equals(type))
			throw new GdxRuntimeException("Asset with name '" + fileName + "' already loaded, but has different type (expected: "
				+ ClassReflection.getSimpleName(type) + ", found: " + ClassReflection.getSimpleName(otherType) + ")");

		toLoad++;
		AssetDescriptor assetDesc = new AssetDescriptor(fileName, type, parameter);
		loadQueue.add(assetDesc);
		log.debug("Queued: " + assetDesc);
	}

	
	public synchronized void load (AssetDescriptor desc) {
		load(desc.fileName, desc.type, desc.params);
	}

	
	public synchronized boolean update () {
		try {
			if (tasks.size() == 0) {
								while (loadQueue.size != 0 && tasks.size() == 0) {
					nextTask();
				}
								if (tasks.size() == 0) return true;
			}
			return updateTask() && loadQueue.size == 0 && tasks.size() == 0;
		} catch (Throwable t) {
			handleTaskError(t);
			return loadQueue.size == 0;
		}
	}

	
	public boolean update (int millis) {
		long endTime = TimeUtils.millis() + millis;
		while (true) {
			boolean done = update();
			if (done || TimeUtils.millis() > endTime) return done;
			ThreadUtils.yield();
		}
	}

	
	public void finishLoading () {
		log.debug("Waiting for loading to complete...");
		while (!update())
			ThreadUtils.yield();
		log.debug("Loading complete.");
	}

	
	public void finishLoadingAsset (String fileName) {
		log.debug("Waiting for asset to be loaded: " + fileName);
		while (!isLoaded(fileName)) {
			update();
			ThreadUtils.yield();
		}
		log.debug("Asset loaded: " + fileName);
	}

	synchronized void injectDependencies (String parentAssetFilename, Array<AssetDescriptor> dependendAssetDescs) {
		ObjectSet<String> injected = this.injected;
		for (AssetDescriptor desc : dependendAssetDescs) {
			if (injected.contains(desc.fileName)) continue; 			injected.add(desc.fileName);
			injectDependency(parentAssetFilename, desc);
		}
		injected.clear();
	}

	private synchronized void injectDependency (String parentAssetFilename, AssetDescriptor dependendAssetDesc) {
				Array<String> dependencies = assetDependencies.get(parentAssetFilename);
		if (dependencies == null) {
			dependencies = new Array();
			assetDependencies.put(parentAssetFilename, dependencies);
		}
		dependencies.add(dependendAssetDesc.fileName);

				if (isLoaded(dependendAssetDesc.fileName)) {
			log.debug("Dependency already loaded: " + dependendAssetDesc);
			Class type = assetTypes.get(dependendAssetDesc.fileName);
			RefCountedContainer assetRef = assets.get(type).get(dependendAssetDesc.fileName);
			assetRef.incRefCount();
			incrementRefCountedDependencies(dependendAssetDesc.fileName);
		}
				else {
			log.info("Loading dependency: " + dependendAssetDesc);
			addTask(dependendAssetDesc);
		}
	}

	
	private void nextTask () {
		AssetDescriptor assetDesc = loadQueue.removeIndex(0);

				if (isLoaded(assetDesc.fileName)) {
			log.debug("Already loaded: " + assetDesc);
			Class type = assetTypes.get(assetDesc.fileName);
			RefCountedContainer assetRef = assets.get(type).get(assetDesc.fileName);
			assetRef.incRefCount();
			incrementRefCountedDependencies(assetDesc.fileName);
			if (assetDesc.params != null && assetDesc.params.loadedCallback != null) {
				assetDesc.params.loadedCallback.finishedLoading(this, assetDesc.fileName, assetDesc.type);
			}
			loaded++;
		} else {
						log.info("Loading: " + assetDesc);
			addTask(assetDesc);
		}
	}

	
	private void addTask (AssetDescriptor assetDesc) {
		AssetLoader loader = getLoader(assetDesc.type, assetDesc.fileName);
		if (loader == null) throw new GdxRuntimeException("No loader for type: " + ClassReflection.getSimpleName(assetDesc.type));
		tasks.push(new AssetLoadingTask(this, assetDesc, loader, executor));
		peakTasks++;
	}

	
	protected <T> void addAsset (final String fileName, Class<T> type, T asset) {
				assetTypes.put(fileName, type);

				ObjectMap<String, RefCountedContainer> typeToAssets = assets.get(type);
		if (typeToAssets == null) {
			typeToAssets = new ObjectMap<String, RefCountedContainer>();
			assets.put(type, typeToAssets);
		}
		typeToAssets.put(fileName, new RefCountedContainer(asset));
	}

	
	private boolean updateTask () {
		AssetLoadingTask task = tasks.peek();

		boolean complete = true;
		try {
			complete = task.cancel || task.update();
		} catch (RuntimeException ex) {
			task.cancel = true;
			taskFailed(task.assetDesc, ex);
		}

				if (complete) {
						if (tasks.size() == 1)  {
				loaded++;
				peakTasks = 0;
			}
			tasks.pop();

			if (task.cancel) return true;

			addAsset(task.assetDesc.fileName, task.assetDesc.type, task.getAsset());

						if (task.assetDesc.params != null && task.assetDesc.params.loadedCallback != null) {
				task.assetDesc.params.loadedCallback.finishedLoading(this, task.assetDesc.fileName, task.assetDesc.type);
			}

			long endTime = TimeUtils.nanoTime();
			log.debug("Loaded: " + (endTime - task.startTime) / 1000000f + "ms " + task.assetDesc);

			return true;
		}
		return false;
	}

	
	protected void taskFailed (AssetDescriptor assetDesc, RuntimeException ex) {
		throw ex;
	}

	private void incrementRefCountedDependencies (String parent) {
		Array<String> dependencies = assetDependencies.get(parent);
		if (dependencies == null) return;

		for (String dependency : dependencies) {
			Class type = assetTypes.get(dependency);
			RefCountedContainer assetRef = assets.get(type).get(dependency);
			assetRef.incRefCount();
			incrementRefCountedDependencies(dependency);
		}
	}

	
	private void handleTaskError (Throwable t) {
		log.error("Error loading asset.", t);

		if (tasks.isEmpty()) throw new GdxRuntimeException(t);

				AssetLoadingTask task = tasks.pop();
		AssetDescriptor assetDesc = task.assetDesc;

				if (task.dependenciesLoaded && task.dependencies != null) {
			for (AssetDescriptor desc : task.dependencies) {
				unload(desc.fileName);
			}
		}

				tasks.clear();

				if (listener != null) {
			listener.error(assetDesc, t);
		} else {
			throw new GdxRuntimeException(t);
		}
	}

	
	public synchronized <T, P extends AssetLoaderParameters<T>> void setLoader (Class<T> type, AssetLoader<T, P> loader) {
		setLoader(type, null, loader);
	}

	
	public synchronized <T, P extends AssetLoaderParameters<T>> void setLoader (Class<T> type, String suffix,
		AssetLoader<T, P> loader) {
		if (type == null) throw new IllegalArgumentException("type cannot be null.");
		if (loader == null) throw new IllegalArgumentException("loader cannot be null.");
		log.debug("Loader set: " + ClassReflection.getSimpleName(type) + " -> " + ClassReflection.getSimpleName(loader.getClass()));
		ObjectMap<String, AssetLoader> loaders = this.loaders.get(type);
		if (loaders == null) this.loaders.put(type, loaders = new ObjectMap<String, AssetLoader>());
		loaders.put(suffix == null ? "" : suffix, loader);
	}

	
	public synchronized int getLoadedAssets () {
		return assetTypes.size;
	}

	
	public synchronized int getQueuedAssets () {
		return loadQueue.size + tasks.size();
	}

	
	public synchronized float getProgress () {
		if (toLoad == 0) return 1;
		float fractionalLoaded = (float)loaded;
		if (peakTasks > 0) {
			fractionalLoaded += ((peakTasks - tasks.size()) / (float)peakTasks);
		}
		return Math.min(1, fractionalLoaded / (float)toLoad);
	}

	
	public synchronized void setErrorListener (AssetErrorListener listener) {
		this.listener = listener;
	}

	
	@Override
	public synchronized void dispose () {
		log.debug("Disposing.");
		clear();
		executor.dispose();
	}

	
	public synchronized void clear () {
		loadQueue.clear();
		while (!update())
			;

		ObjectIntMap<String> dependencyCount = new ObjectIntMap<String>();
		while (assetTypes.size > 0) {
						dependencyCount.clear();
			Array<String> assets = assetTypes.keys().toArray();
			for (String asset : assets) {
				dependencyCount.put(asset, 0);
			}

			for (String asset : assets) {
				Array<String> dependencies = assetDependencies.get(asset);
				if (dependencies == null) continue;
				for (String dependency : dependencies) {
					int count = dependencyCount.get(dependency, 0);
					count++;
					dependencyCount.put(dependency, count);
				}
			}

						for (String asset : assets) {
				if (dependencyCount.get(asset, 0) == 0) {
					unload(asset);
				}
			}
		}

		this.assets.clear();
		this.assetTypes.clear();
		this.assetDependencies.clear();
		this.loaded = 0;
		this.toLoad = 0;
		this.peakTasks = 0;
		this.loadQueue.clear();
		this.tasks.clear();
	}

	
	public Logger getLogger () {
		return log;
	}

	public void setLogger (Logger logger) {
		log = logger;
	}

	
	public synchronized int getReferenceCount (String fileName) {
		Class type = assetTypes.get(fileName);
		if (type == null) throw new GdxRuntimeException("Asset not loaded: " + fileName);
		return assets.get(type).get(fileName).getRefCount();
	}

	
	public synchronized void setReferenceCount (String fileName, int refCount) {
		Class type = assetTypes.get(fileName);
		if (type == null) throw new GdxRuntimeException("Asset not loaded: " + fileName);
		assets.get(type).get(fileName).setRefCount(refCount);
	}

	
	public synchronized String getDiagnostics () {
		StringBuffer buffer = new StringBuffer();
		for (String fileName : assetTypes.keys()) {
			buffer.append(fileName);
			buffer.append(", ");

			Class type = assetTypes.get(fileName);
			RefCountedContainer assetRef = assets.get(type).get(fileName);
			Array<String> dependencies = assetDependencies.get(fileName);

			buffer.append(ClassReflection.getSimpleName(type));

			buffer.append(", refs: ");
			buffer.append(assetRef.getRefCount());

			if (dependencies != null) {
				buffer.append(", deps: [");
				for (String dep : dependencies) {
					buffer.append(dep);
					buffer.append(",");
				}
				buffer.append("]");
			}
			buffer.append("\n");
		}
		return buffer.toString();
	}

	
	public synchronized Array<String> getAssetNames () {
		return assetTypes.keys().toArray();
	}

	
	public synchronized Array<String> getDependencies (String fileName) {
		return assetDependencies.get(fileName);
	}

	
	public synchronized Class getAssetType (String fileName) {
		return assetTypes.get(fileName);
	}

}
