

package com.badlogic.gdx.tests.g3d.shadows.system;

import java.util.Set;

import com.badlogic.gdx.graphics.Camera;
import com.badlogic.gdx.graphics.Cubemap.CubemapSide;
import com.badlogic.gdx.graphics.g3d.RenderableProvider;
import com.badlogic.gdx.graphics.g3d.environment.DirectionalLight;
import com.badlogic.gdx.graphics.g3d.environment.PointLight;
import com.badlogic.gdx.graphics.g3d.environment.SpotLight;
import com.badlogic.gdx.graphics.g3d.utils.ShaderProvider;


public interface ShadowSystem {

	
	public void init ();

	
	public int getPassQuantity ();

	
	public ShaderProvider getPassShaderProvider (int n);

	
	public ShaderProvider getShaderProvider ();

	
	public void addLight (SpotLight spot);

	
	public void addLight (DirectionalLight dir);

	
	public void addLight (PointLight point);

	
	public void addLight (PointLight point, Set<CubemapSide> sides);

	
	public void removeLight (SpotLight spot);

	
	public void removeLight (DirectionalLight dir);

	
	public void removeLight (PointLight point);

	
	public boolean hasLight (SpotLight spot);

	
	public boolean hasLight (DirectionalLight dir);

	
	public boolean hasLight (PointLight point);

	
	public void update ();

	
	public <T extends RenderableProvider> void begin (Camera camera, Iterable<T> renderableProviders);

	
	public void begin (int n);

	
	public Camera next ();

	
	public void end ();

	
	public void end (int n);
}
