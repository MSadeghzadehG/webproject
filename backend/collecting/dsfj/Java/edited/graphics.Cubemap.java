

package com.badlogic.gdx.graphics;

import java.util.HashMap;
import java.util.Map;

import com.badlogic.gdx.Application;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.assets.AssetLoaderParameters.LoadedCallback;
import com.badlogic.gdx.assets.AssetManager;
import com.badlogic.gdx.assets.loaders.AssetLoader;
import com.badlogic.gdx.assets.loaders.CubemapLoader.CubemapParameter;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.Pixmap.Format;
import com.badlogic.gdx.graphics.Texture.TextureFilter;
import com.badlogic.gdx.graphics.Texture.TextureWrap;
import com.badlogic.gdx.graphics.glutils.FacedCubemapData;
import com.badlogic.gdx.graphics.glutils.PixmapTextureData;
import com.badlogic.gdx.math.Vector3;
import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.GdxRuntimeException;


public class Cubemap extends GLTexture {
	private static AssetManager assetManager;
	final static Map<Application, Array<Cubemap>> managedCubemaps = new HashMap<Application, Array<Cubemap>>();

	
	public enum CubemapSide {
		
		PositiveX(0, GL20.GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, -1, 0, 1, 0, 0),
		
		NegativeX(1, GL20.GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, -1, 0, -1, 0, 0),
		
		PositiveY(2, GL20.GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 0, 1, 0, 1, 0),
		
		NegativeY(3, GL20.GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0, -1, 0, -1, 0),
		
		PositiveZ(4, GL20.GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, -1, 0, 0, 0, 1),
		
		NegativeZ(5, GL20.GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, -1, 0, 0, 0, -1);

		
		public final int index;
		
		public final int glEnum;
		
		public final Vector3 up;
		
		public final Vector3 direction;

		CubemapSide (int index, int glEnum, float upX, float upY, float upZ, float directionX, float directionY, float directionZ) {
			this.index = index;
			this.glEnum = glEnum;
			this.up = new Vector3(upX, upY, upZ);
			this.direction = new Vector3(directionX, directionY, directionZ);
		}

		
		public int getGLEnum () {
			return glEnum;
		}

		
		public Vector3 getUp (Vector3 out) {
			return out.set(up);
		}

		
		public Vector3 getDirection (Vector3 out) {
			return out.set(direction);
		}
	}

	protected CubemapData data;

	
	public Cubemap (CubemapData data) {
		super(GL20.GL_TEXTURE_CUBE_MAP);
		this.data = data;
		load(data);
	}

	
	public Cubemap (FileHandle positiveX, FileHandle negativeX, FileHandle positiveY, FileHandle negativeY, FileHandle positiveZ,
		FileHandle negativeZ) {
		this(positiveX, negativeX, positiveY, negativeY, positiveZ, negativeZ, false);
	}

	
	public Cubemap (FileHandle positiveX, FileHandle negativeX, FileHandle positiveY, FileHandle negativeY, FileHandle positiveZ,
		FileHandle negativeZ, boolean useMipMaps) {
		this(TextureData.Factory.loadFromFile(positiveX, useMipMaps), TextureData.Factory.loadFromFile(negativeX, useMipMaps),
			TextureData.Factory.loadFromFile(positiveY, useMipMaps), TextureData.Factory.loadFromFile(negativeY, useMipMaps),
			TextureData.Factory.loadFromFile(positiveZ, useMipMaps), TextureData.Factory.loadFromFile(negativeZ, useMipMaps));
	}

	
	public Cubemap (Pixmap positiveX, Pixmap negativeX, Pixmap positiveY, Pixmap negativeY, Pixmap positiveZ, Pixmap negativeZ) {
		this(positiveX, negativeX, positiveY, negativeY, positiveZ, negativeZ, false);
	}

	
	public Cubemap (Pixmap positiveX, Pixmap negativeX, Pixmap positiveY, Pixmap negativeY, Pixmap positiveZ, Pixmap negativeZ,
		boolean useMipMaps) {
		this(positiveX == null ? null : new PixmapTextureData(positiveX, null, useMipMaps, false), negativeX == null ? null
			: new PixmapTextureData(negativeX, null, useMipMaps, false), positiveY == null ? null : new PixmapTextureData(positiveY,
			null, useMipMaps, false), negativeY == null ? null : new PixmapTextureData(negativeY, null, useMipMaps, false),
			positiveZ == null ? null : new PixmapTextureData(positiveZ, null, useMipMaps, false), negativeZ == null ? null
				: new PixmapTextureData(negativeZ, null, useMipMaps, false));
	}

	
	public Cubemap (int width, int height, int depth, Format format) {
		this(new PixmapTextureData(new Pixmap(depth, height, format), null, false, true), new PixmapTextureData(new Pixmap(depth,
			height, format), null, false, true), new PixmapTextureData(new Pixmap(width, depth, format), null, false, true),
			new PixmapTextureData(new Pixmap(width, depth, format), null, false, true), new PixmapTextureData(new Pixmap(width,
				height, format), null, false, true), new PixmapTextureData(new Pixmap(width, height, format), null, false, true));
	}

	
	public Cubemap (TextureData positiveX, TextureData negativeX, TextureData positiveY, TextureData negativeY,
		TextureData positiveZ, TextureData negativeZ) {
		super(GL20.GL_TEXTURE_CUBE_MAP);
		minFilter = TextureFilter.Nearest;
		magFilter = TextureFilter.Nearest;
		uWrap = TextureWrap.ClampToEdge;
		vWrap = TextureWrap.ClampToEdge;
		data = new FacedCubemapData(positiveX, negativeX, positiveY, negativeY, positiveZ, negativeZ);
		load(data);
	}

	
	public void load (CubemapData data) {
		if (!data.isPrepared()) data.prepare();
		bind();
		unsafeSetFilter(minFilter, magFilter, true);
		unsafeSetWrap(uWrap, vWrap, true);
		data.consumeCubemapData();
		Gdx.gl.glBindTexture(glTarget, 0);
	}

	public CubemapData getCubemapData () {
		return data;
	}

	@Override
	public boolean isManaged () {
		return data.isManaged();
	}

	@Override
	protected void reload () {
		if (!isManaged()) throw new GdxRuntimeException("Tried to reload an unmanaged Cubemap");
		glHandle = Gdx.gl.glGenTexture();
		load(data);
	}

	@Override
	public int getWidth () {
		return data.getWidth();
	}

	@Override
	public int getHeight () {
		return data.getHeight();
	}

	@Override
	public int getDepth () {
		return 0;
	}

	
	@Override
	public void dispose () {
										if (glHandle == 0) return;
		delete();
		if (data.isManaged()) if (managedCubemaps.get(Gdx.app) != null) managedCubemaps.get(Gdx.app).removeValue(this, true);
	}

	private static void addManagedCubemap (Application app, Cubemap cubemap) {
		Array<Cubemap> managedCubemapArray = managedCubemaps.get(app);
		if (managedCubemapArray == null) managedCubemapArray = new Array<Cubemap>();
		managedCubemapArray.add(cubemap);
		managedCubemaps.put(app, managedCubemapArray);
	}

	
	public static void clearAllCubemaps (Application app) {
		managedCubemaps.remove(app);
	}

	
	public static void invalidateAllCubemaps (Application app) {
		Array<Cubemap> managedCubemapArray = managedCubemaps.get(app);
		if (managedCubemapArray == null) return;

		if (assetManager == null) {
			for (int i = 0; i < managedCubemapArray.size; i++) {
				Cubemap cubemap = managedCubemapArray.get(i);
				cubemap.reload();
			}
		} else {
												assetManager.finishLoading();

									Array<Cubemap> cubemaps = new Array<Cubemap>(managedCubemapArray);
			for (Cubemap cubemap : cubemaps) {
				String fileName = assetManager.getAssetFileName(cubemap);
				if (fileName == null) {
					cubemap.reload();
				} else {
																									final int refCount = assetManager.getReferenceCount(fileName);
					assetManager.setReferenceCount(fileName, 0);
					cubemap.glHandle = 0;

															CubemapParameter params = new CubemapParameter();
					params.cubemapData = cubemap.getCubemapData();
					params.minFilter = cubemap.getMinFilter();
					params.magFilter = cubemap.getMagFilter();
					params.wrapU = cubemap.getUWrap();
					params.wrapV = cubemap.getVWrap();
					params.cubemap = cubemap; 					params.loadedCallback = new LoadedCallback() {
						@Override
						public void finishedLoading (AssetManager assetManager, String fileName, Class type) {
							assetManager.setReferenceCount(fileName, refCount);
						}
					};

										assetManager.unload(fileName);
					cubemap.glHandle = Gdx.gl.glGenTexture();
					assetManager.load(fileName, Cubemap.class, params);
				}
			}
			managedCubemapArray.clear();
			managedCubemapArray.addAll(cubemaps);
		}
	}

	
	public static void setAssetManager (AssetManager manager) {
		Cubemap.assetManager = manager;
	}

	public static String getManagedStatus () {
		StringBuilder builder = new StringBuilder();
		builder.append("Managed cubemap/app: { ");
		for (Application app : managedCubemaps.keySet()) {
			builder.append(managedCubemaps.get(app).size);
			builder.append(" ");
		}
		builder.append("}");
		return builder.toString();
	}

	
	public static int getNumManagedCubemaps () {
		return managedCubemaps.get(Gdx.app).size;
	}

}
