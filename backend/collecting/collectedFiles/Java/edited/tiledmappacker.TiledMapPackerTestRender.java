

package com.badlogic.gdx.tiledmappacker;

import java.io.File;

import com.badlogic.gdx.ApplicationAdapter;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.Input.Keys;
import com.badlogic.gdx.assets.loaders.resolvers.InternalFileHandleResolver;
import com.badlogic.gdx.backends.lwjgl.LwjglApplication;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.OrthographicCamera;
import com.badlogic.gdx.maps.tiled.AtlasTmxMapLoader;
import com.badlogic.gdx.maps.tiled.TiledMap;
import com.badlogic.gdx.maps.tiled.renderers.OrthogonalTiledMapRenderer;
import com.badlogic.gdx.utils.viewport.FitViewport;
import com.badlogic.gdx.utils.viewport.Viewport;


public class TiledMapPackerTestRender extends ApplicationAdapter {

			
	private final boolean DELETE_DELETEME_FOLDER_ON_EXIT = false; 	private final static String MAP_PATH = "../../tests/gdx-tests-android/assets/data/maps/tiled-atlas-processed/deleteMe/";

	private final String MAP_NAME = "test.tmx";
	private final String TMX_LOC = MAP_PATH + MAP_NAME;
	private final boolean CENTER_CAM = true;
	private final float WORLD_WIDTH = 32;
	private final float WORLD_HEIGHT = 18;
	private final float PIXELS_PER_METER = 32;
	private final float UNIT_SCALE = 1f / PIXELS_PER_METER;
	private AtlasTmxMapLoader.AtlasTiledMapLoaderParameters params;
	private AtlasTmxMapLoader atlasTmxMapLoader;
	private TiledMap map;
	private Viewport viewport;
	private OrthogonalTiledMapRenderer mapRenderer;
	private OrthographicCamera cam;

	@Override
	public void create () {
		atlasTmxMapLoader = new AtlasTmxMapLoader(new InternalFileHandleResolver());
		params = new AtlasTmxMapLoader.AtlasTiledMapLoaderParameters();

		params.generateMipMaps = false;
		params.convertObjectToTileSpace = false;
		params.flipY = true;

		viewport = new FitViewport(WORLD_WIDTH, WORLD_HEIGHT);
		cam = (OrthographicCamera)viewport.getCamera();

		map = atlasTmxMapLoader.load(TMX_LOC, params);
		mapRenderer = new OrthogonalTiledMapRenderer(map, UNIT_SCALE);
	}

	@Override
	public void render () {
		Gdx.gl.glClearColor(0, 0, 0, 1f);
		Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT);

		viewport.apply();
		mapRenderer.setView(cam);
		mapRenderer.render();

		if (Gdx.input.isKeyPressed(Keys.ESCAPE)) {
			if (DELETE_DELETEME_FOLDER_ON_EXIT) {
				FileHandle deleteMeHandle = Gdx.files.local(MAP_PATH);
				deleteMeHandle.deleteDirectory();
			}

			dispose();
			Gdx.app.exit();
		}
	}

	@Override
	public void resize (int width, int height) {
		viewport.update(width, height, CENTER_CAM);
	}

	@Override
	public void dispose () {
		map.dispose();
	}

	public static void main (String[] args) throws Exception {
		File file = new File(MAP_PATH);
		if (!file.exists()) {
			System.out.println("Please run TiledMapPackerTest.");
			return;
		}
		new LwjglApplication(new TiledMapPackerTestRender(), "", 640, 480);
	}
}
