

package com.badlogic.gdx.graphics.g2d;

import java.io.BufferedReader;
import java.io.IOException;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.assets.AssetDescriptor;
import com.badlogic.gdx.assets.AssetLoaderParameters;
import com.badlogic.gdx.assets.AssetManager;
import com.badlogic.gdx.assets.loaders.FileHandleResolver;
import com.badlogic.gdx.assets.loaders.SynchronousAssetLoader;
import com.badlogic.gdx.assets.loaders.resolvers.InternalFileHandleResolver;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.PolygonRegionLoader.PolygonRegionParameters;
import com.badlogic.gdx.math.EarClippingTriangulator;
import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.StreamUtils;


public class PolygonRegionLoader extends SynchronousAssetLoader<PolygonRegion, PolygonRegionParameters> {

	public static class PolygonRegionParameters extends AssetLoaderParameters<PolygonRegion> {

		
		public String texturePrefix = "i ";

		
		public int readerBuffer = 1024;

		
		public String[] textureExtensions = new String[] {"png", "PNG", "jpeg", "JPEG", "jpg", "JPG", "cim", "CIM", "etc1", "ETC1",
			"ktx", "KTX", "zktx", "ZKTX"};

	}

	private PolygonRegionParameters defaultParameters = new PolygonRegionParameters();

	private EarClippingTriangulator triangulator = new EarClippingTriangulator();
	
	public PolygonRegionLoader() {
		this(new InternalFileHandleResolver());
	}
	
	public PolygonRegionLoader (FileHandleResolver resolver) {
		super(resolver);
	}

	@Override
	public PolygonRegion load (AssetManager manager, String fileName, FileHandle file, PolygonRegionParameters parameter) {
		Texture texture = manager.get(manager.getDependencies(fileName).first());
		return load(new TextureRegion(texture), file);
	}

	
	@Override
	public Array<AssetDescriptor> getDependencies (String fileName, FileHandle file, PolygonRegionParameters params) {
		if (params == null) params = defaultParameters;
		String image = null;
		try {
			BufferedReader reader = file.reader(params.readerBuffer);
			for (String line = reader.readLine(); line != null; line = reader.readLine())
				if (line.startsWith(params.texturePrefix)) {
					image = line.substring(params.texturePrefix.length());
					break;
				}
			reader.close();
		} catch (IOException e) {
			throw new GdxRuntimeException("Error reading " + fileName, e);
		}

		if (image == null && params.textureExtensions != null) for (String extension : params.textureExtensions) {
			FileHandle sibling = file.sibling(file.nameWithoutExtension().concat("." + extension));
			if (sibling.exists()) image = sibling.name();
		}

		if (image != null) {
			Array<AssetDescriptor> deps = new Array<AssetDescriptor>(1);
			deps.add(new AssetDescriptor<Texture>(file.sibling(image), Texture.class));
			return deps;
		}

		return null;
	}

	
	public PolygonRegion load (TextureRegion textureRegion, FileHandle file) {
		BufferedReader reader = file.reader(256);
		try {
			while (true) {
				String line = reader.readLine();
				if (line == null) break;
				if (line.startsWith("s")) {
										String[] polygonStrings = line.substring(1).trim().split(",");
					float[] vertices = new float[polygonStrings.length];
					for (int i = 0, n = vertices.length; i < n; i++)
						vertices[i] = Float.parseFloat(polygonStrings[i]);
										return new PolygonRegion(textureRegion, vertices, triangulator.computeTriangles(vertices).toArray());
				}
			}
		} catch (IOException ex) {
			throw new GdxRuntimeException("Error reading polygon shape file: " + file, ex);
		} finally {
			StreamUtils.closeQuietly(reader);
		}
		throw new GdxRuntimeException("Polygon shape not found: " + file);
	}
	
}
