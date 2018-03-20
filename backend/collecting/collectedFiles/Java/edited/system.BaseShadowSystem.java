

package com.badlogic.gdx.tests.g3d.shadows.system;

import java.util.EnumSet;
import java.util.Set;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.Camera;
import com.badlogic.gdx.graphics.Cubemap;
import com.badlogic.gdx.graphics.Cubemap.CubemapSide;
import com.badlogic.gdx.graphics.OrthographicCamera;
import com.badlogic.gdx.graphics.PerspectiveCamera;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g2d.TextureRegion;
import com.badlogic.gdx.graphics.g3d.RenderableProvider;
import com.badlogic.gdx.graphics.g3d.environment.BaseLight;
import com.badlogic.gdx.graphics.g3d.environment.DirectionalLight;
import com.badlogic.gdx.graphics.g3d.environment.PointLight;
import com.badlogic.gdx.graphics.g3d.environment.SpotLight;
import com.badlogic.gdx.graphics.g3d.utils.ShaderProvider;
import com.badlogic.gdx.graphics.glutils.FrameBuffer;
import com.badlogic.gdx.tests.g3d.shadows.utils.AABBNearFarAnalyzer;
import com.badlogic.gdx.tests.g3d.shadows.utils.BoundingSphereDirectionalAnalyzer;
import com.badlogic.gdx.tests.g3d.shadows.utils.DirectionalAnalyzer;
import com.badlogic.gdx.tests.g3d.shadows.utils.FixedShadowMapAllocator;
import com.badlogic.gdx.tests.g3d.shadows.utils.FrustumLightFilter;
import com.badlogic.gdx.tests.g3d.shadows.utils.LightFilter;
import com.badlogic.gdx.tests.g3d.shadows.utils.NearFarAnalyzer;
import com.badlogic.gdx.tests.g3d.shadows.utils.ShadowMapAllocator;
import com.badlogic.gdx.tests.g3d.shadows.utils.ShadowMapAllocator.ShadowMapRegion;
import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.ObjectMap;
import com.badlogic.gdx.utils.ObjectMap.Entries;


public abstract class BaseShadowSystem implements ShadowSystem, Disposable {
	
	public static class LightProperties {
		public Camera camera;
		public TextureRegion region = new TextureRegion();

		public LightProperties (Camera camera) {
			this.camera = camera;
		}
	}

	
	public static class PointLightProperties {
		public ObjectMap<CubemapSide, LightProperties> properties = new ObjectMap<CubemapSide, LightProperties>(6);
	}

	
	protected Camera camera;
	
	protected Iterable<RenderableProvider> renderableProviders;
	
	protected ObjectMap<SpotLight, LightProperties> spotCameras = new ObjectMap<SpotLight, LightProperties>();
	
	protected ObjectMap<DirectionalLight, LightProperties> dirCameras = new ObjectMap<DirectionalLight, LightProperties>();
	
	protected ObjectMap<PointLight, PointLightProperties> pointCameras = new ObjectMap<PointLight, PointLightProperties>();
	
	protected NearFarAnalyzer nearFarAnalyzer;
	
	protected ShadowMapAllocator allocator;
	
	protected DirectionalAnalyzer directionalAnalyzer;
	
	protected LightFilter lightFilter;
	
	protected FrameBuffer[] frameBuffers;
	
	protected int currentPass = -1;
	
	protected Entries<SpotLight, LightProperties> spotCameraIterator;
	protected Entries<DirectionalLight, LightProperties> dirCameraIterator;
	protected Entries<PointLight, PointLightProperties> pointCameraIterator;
	
	protected int currentPointSide;
	protected PointLightProperties currentPointProperties;
	
	protected ShaderProvider[] passShaderProviders;
	protected ShaderProvider mainShaderProvider;
	
	protected LightProperties currentLightProperties;
	protected BaseLight currentLight;

	
	public BaseShadowSystem (NearFarAnalyzer nearFarAnalyzer, ShadowMapAllocator allocator,
		DirectionalAnalyzer directionalAnalyzer, LightFilter lightFilter) {
		this.nearFarAnalyzer = nearFarAnalyzer;
		this.allocator = allocator;
		this.directionalAnalyzer = directionalAnalyzer;
		this.lightFilter = lightFilter;
	}

	
	public BaseShadowSystem () {
		this(new AABBNearFarAnalyzer(), new FixedShadowMapAllocator(FixedShadowMapAllocator.QUALITY_MED,
			FixedShadowMapAllocator.QUANTITY_MAP_MED), new BoundingSphereDirectionalAnalyzer(), new FrustumLightFilter());
	}

	
	@Override
	public void init () {
		frameBuffers = new FrameBuffer[getPassQuantity()];
		passShaderProviders = new ShaderProvider[getPassQuantity()];

		for (int i = 0; i < getPassQuantity(); i++) {
			init(i);
		}
	};

	
	protected abstract void init (int n);

	
	@Override
	public abstract int getPassQuantity ();

	@Override
	public ShaderProvider getPassShaderProvider (int n) {
		return passShaderProviders[n];
	}

	@Override
	public ShaderProvider getShaderProvider () {
		return mainShaderProvider;
	}

	@Override
	public void addLight (SpotLight spot) {
		PerspectiveCamera camera = new PerspectiveCamera(spot.cutoffAngle, 0, 0);
		camera.position.set(spot.position);
		camera.direction.set(spot.direction);
		camera.near = 1;
		camera.far = 100;
		camera.up.set(camera.direction.y, camera.direction.z, camera.direction.x);

		spotCameras.put(spot, new LightProperties(camera));
	}

	@Override
	public void addLight (DirectionalLight dir) {
		OrthographicCamera camera = new OrthographicCamera();
		camera.direction.set(dir.direction);
		camera.near = 1;
		camera.far = 100;

		dirCameras.put(dir, new LightProperties(camera));
	}

	@Override
	public void addLight (PointLight point) {
		addLight(point, EnumSet.of(CubemapSide.PositiveX, CubemapSide.NegativeX, CubemapSide.PositiveY, CubemapSide.NegativeY,
			CubemapSide.PositiveZ, CubemapSide.NegativeZ));
	}

	@Override
	public void addLight (PointLight point, Set<CubemapSide> sides) {
		PointLightProperties plProperty = new PointLightProperties();
		for (int i = 0; i < 6; i++) {
			CubemapSide cubemapSide = Cubemap.CubemapSide.values()[i];
			if (sides.contains(cubemapSide)) {
				PerspectiveCamera camera = new PerspectiveCamera(90, 0, 0);
				camera.position.set(point.position);
				camera.direction.set(cubemapSide.direction);
				camera.up.set(cubemapSide.up);
				camera.near = 1;
				camera.far = 100;

				LightProperties p = new LightProperties(camera);
				plProperty.properties.put(cubemapSide, p);
			}
		}
		pointCameras.put(point, plProperty);
	}

	@Override
	public void removeLight (SpotLight spot) {
		spotCameras.remove(spot);
	}

	@Override
	public void removeLight (DirectionalLight dir) {
		dirCameras.remove(dir);
	}

	@Override
	public void removeLight (PointLight point) {
		pointCameras.remove(point);
	}

	@Override
	public boolean hasLight (SpotLight spot) {
		if (spotCameras.containsKey(spot)) return true;
		return false;
	}

	@Override
	public boolean hasLight (DirectionalLight dir) {
		if (dirCameras.containsKey(dir)) return true;
		return false;
	}

	@Override
	public boolean hasLight (PointLight point) {
		if (pointCameras.containsKey(point)) return true;
		return false;
	}

	@Override
	public void update () {
		for (ObjectMap.Entry<SpotLight, LightProperties> e : spotCameras) {
			e.value.camera.position.set(e.key.position);
			e.value.camera.direction.set(e.key.direction);
			nearFarAnalyzer.analyze(e.key, e.value.camera, renderableProviders);
		}

		for (ObjectMap.Entry<DirectionalLight, LightProperties> e : dirCameras) {
			directionalAnalyzer.analyze(e.key, e.value.camera, camera).update();
		}

		for (ObjectMap.Entry<PointLight, PointLightProperties> e : pointCameras) {
			for (ObjectMap.Entry<CubemapSide, LightProperties> c : e.value.properties) {
				c.value.camera.position.set(e.key.position);
				nearFarAnalyzer.analyze(e.key, c.value.camera, renderableProviders);
			}
		}
	}

	@Override
	public <T extends RenderableProvider> void begin (Camera camera, final Iterable<T> renderableProviders) {
		if (this.renderableProviders != null || this.camera != null) throw new GdxRuntimeException("Call end() first.");

		this.camera = camera;
		this.renderableProviders = (Iterable<RenderableProvider>)renderableProviders;
	}

	@Override
	public void begin (int n) {
		if (n >= passShaderProviders.length)
			throw new GdxRuntimeException("Pass " + n + " doesn't exist in " + getClass().getName());

		currentPass = n;
		spotCameraIterator = spotCameras.iterator();
		dirCameraIterator = dirCameras.iterator();
		pointCameraIterator = pointCameras.iterator();
		currentPointSide = 6;

		beginPass(n);
	}

	
	protected void beginPass (int n) {
		frameBuffers[n].begin();
	};

	@Override
	public void end () {
		this.camera = null;
		this.renderableProviders = null;
		currentPass = -1;
	}

	@Override
	public void end (int n) {
		if (currentPass != n) throw new GdxRuntimeException("Begin " + n + " must be called before end " + n);
		endPass(n);
	}

	
	protected void endPass (int n) {
		frameBuffers[n].end();
	}

	@Override
	public Camera next () {
		LightProperties lp = nextDirectional();
		if (lp != null) return interceptCamera(lp);

		lp = nextSpot();
		if (lp != null) return interceptCamera(lp);

		lp = nextPoint();
		if (lp != null) return interceptCamera(lp);

		return null;
	}

	
	protected Camera interceptCamera (LightProperties lp) {
		return lp.camera;
	}

	protected LightProperties nextDirectional () {
		if (!dirCameraIterator.hasNext()) return null;

		ObjectMap.Entry<DirectionalLight, LightProperties> e = dirCameraIterator.next();
		currentLight = e.key;
		currentLightProperties = e.value;
		LightProperties lp = e.value;
		processViewport(lp, false);
		return lp;
	}

	protected LightProperties nextSpot () {
		if (!spotCameraIterator.hasNext()) return null;

		ObjectMap.Entry<SpotLight, LightProperties> e = spotCameraIterator.next();
		currentLight = e.key;
		currentLightProperties = e.value;
		LightProperties lp = e.value;

		if (!lightFilter.filter(spotCameras.findKey(lp, true), lp.camera, this.camera)) {
			return nextSpot();
		}

		processViewport(lp, true);
		return lp;
	}

	protected LightProperties nextPoint () {
		if (!pointCameraIterator.hasNext() && currentPointSide > 5) return null;

		if (currentPointSide > 5) currentPointSide = 0;

		if (currentPointSide == 0) {
			ObjectMap.Entry<PointLight, PointLightProperties> e = pointCameraIterator.next();
			currentLight = e.key;
			currentPointProperties = e.value;
		}

		if (currentPointProperties.properties.containsKey(Cubemap.CubemapSide.values()[currentPointSide])) {
			LightProperties lp = currentPointProperties.properties.get(Cubemap.CubemapSide.values()[currentPointSide]);
			currentLightProperties = lp;
			currentPointSide += 1;

			if (!lightFilter.filter(pointCameras.findKey(currentPointProperties, true), lp.camera, this.camera)) {
				return nextPoint();
			}

			processViewport(lp, true);
			return lp;
		}

		currentPointSide += 1;
		return nextPoint();
	}

	
	protected void processViewport (LightProperties lp, boolean cameraViewport) {
		Camera camera = lp.camera;
		ShadowMapRegion r = allocator.nextResult(currentLight);

		if (r == null) return;

		TextureRegion region = lp.region;
		region.setTexture(frameBuffers[currentPass].getColorBufferTexture());
		
						Gdx.gl.glViewport(r.x, r.y, r.width, r.height);
		Gdx.gl.glScissor(r.x + 1, r.y + 1, r.width - 2, r.height - 2);
		region.setRegion(r.x, r.y, r.width, r.height);

		if (cameraViewport) {
			camera.viewportHeight = r.height;
			camera.viewportWidth = r.width;
			camera.update();
		}
	}

	public ObjectMap<DirectionalLight, LightProperties> getDirectionalCameras () {
		return dirCameras;
	}

	public ObjectMap<SpotLight, LightProperties> getSpotCameras () {
		return spotCameras;
	}

	public ObjectMap<PointLight, PointLightProperties> getPointCameras () {
		return pointCameras;
	}

	public Texture getTexture (int n) {
		if (n >= getPassQuantity()) throw new GdxRuntimeException("Can't get texture " + n);
		return frameBuffers[n].getColorBufferTexture();
	}

	public LightProperties getCurrentLightProperties () {
		return currentLightProperties;
	}

	public BaseLight getCurrentLight () {
		return currentLight;
	}

	public int getCurrentPass () {
		return currentPass;
	}

	@Override
	public void dispose () {
		for (int i = 0; i < getPassQuantity(); i++) {
			frameBuffers[i].dispose();
			passShaderProviders[i].dispose();
		}
		mainShaderProvider.dispose();
	}
}
