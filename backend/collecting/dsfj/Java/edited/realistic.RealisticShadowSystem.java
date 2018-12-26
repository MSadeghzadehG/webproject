

package com.badlogic.gdx.tests.g3d.shadows.system.realistic;

import java.util.Set;

import com.badlogic.gdx.graphics.Cubemap.CubemapSide;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.g3d.environment.PointLight;
import com.badlogic.gdx.tests.g3d.shadows.system.FirstPassBaseShadowSystem;
import com.badlogic.gdx.tests.g3d.shadows.utils.DirectionalAnalyzer;
import com.badlogic.gdx.tests.g3d.shadows.utils.LightFilter;
import com.badlogic.gdx.tests.g3d.shadows.utils.NearFarAnalyzer;
import com.badlogic.gdx.tests.g3d.shadows.utils.ShadowMapAllocator;


public class RealisticShadowSystem extends FirstPassBaseShadowSystem {

	
	public static final int PASS_QUANTITY = 1;

	public RealisticShadowSystem () {
		super();
	}

	public RealisticShadowSystem (NearFarAnalyzer nearFarAnalyzer, ShadowMapAllocator allocator,
		DirectionalAnalyzer directionalAnalyzer, LightFilter lightFilter) {
		super(nearFarAnalyzer, allocator, directionalAnalyzer, lightFilter);
	}

	@Override
	public int getPassQuantity () {
		return PASS_QUANTITY;
	}

	@Override
	public void init () {
		super.init();
		mainShaderProvider = new MainShaderProvider(new MainShader.Config(this));
	}

	@Override
	protected void init1 () {
		super.init1();
		passShaderProviders[FIRST_PASS] = new Pass1ShaderProvider();
	}

	
	@Override
	public void addLight (PointLight point, Set<CubemapSide> sides) {
	}

	
	public Texture getTexture () {
		return this.getTexture(FIRST_PASS);
	}

	@Override
	public String toString () {
		return "RealisticShadowSystem";
	}
}
