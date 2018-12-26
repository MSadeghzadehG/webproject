

package com.badlogic.gdx.backends.lwjgl;

import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.util.ArrayList;

import com.badlogic.gdx.Application;
import com.badlogic.gdx.Files;
import com.badlogic.gdx.Files.FileType;
import com.badlogic.gdx.Graphics;
import com.badlogic.gdx.Graphics.DisplayMode;
import com.badlogic.gdx.graphics.Color;
import com.badlogic.gdx.utils.Array;

public class LwjglApplicationConfiguration {
	
	static public boolean disableAudio;

	
	public boolean useGL30 = false;
	
	public int r = 8, g = 8, b = 8, a = 8;
	
	public int depth = 16, stencil = 0;
	
	public int samples = 0;
	
	public int width = 640, height = 480;
	
	public int x = -1, y = -1;
	
	public boolean fullscreen = false;
	
	public int overrideDensity = -1;
	
	public boolean vSyncEnabled = true;
	
	public String title;
	
	public boolean forceExit = true;
	
	public boolean resizable = true;
	
	public int audioDeviceSimultaneousSources = 16;
	
	public int audioDeviceBufferSize = 512;
	
	public int audioDeviceBufferCount = 9;
	public Color initialBackgroundColor = Color.BLACK;
	
	public int foregroundFPS = 60;
	
	public int backgroundFPS = 60;
	
	public boolean allowSoftwareMode = false;
	
	public String preferencesDirectory = ".prefs/";
	
	public Files.FileType preferencesFileType = FileType.External;
	
	public LwjglGraphics.SetDisplayModeCallback setDisplayModeCallback;
	
	public boolean useHDPI = false;

	Array<String> iconPaths = new Array();
	Array<FileType> iconFileTypes = new Array();

	
	public void addIcon (String path, FileType fileType) {
		iconPaths.add(path);
		iconFileTypes.add(fileType);
	}

	
	public void setFromDisplayMode (DisplayMode mode) {
		this.width = mode.width;
		this.height = mode.height;
		if (mode.bitsPerPixel == 16) {
			this.r = 5;
			this.g = 6;
			this.b = 5;
			this.a = 0;
		}
		if (mode.bitsPerPixel == 24) {
			this.r = 8;
			this.g = 8;
			this.b = 8;
			this.a = 0;
		}
		if (mode.bitsPerPixel == 32) {
			this.r = 8;
			this.g = 8;
			this.b = 8;
			this.a = 8;
		}
		this.fullscreen = true;
	}

	protected static class LwjglApplicationConfigurationDisplayMode extends DisplayMode {
		protected LwjglApplicationConfigurationDisplayMode (int width, int height, int refreshRate, int bitsPerPixel) {
			super(width, height, refreshRate, bitsPerPixel);
		}
	}

	public static DisplayMode getDesktopDisplayMode () {
		GraphicsEnvironment genv = GraphicsEnvironment.getLocalGraphicsEnvironment();
		GraphicsDevice device = genv.getDefaultScreenDevice();
		java.awt.DisplayMode mode = device.getDisplayMode();
		return new LwjglApplicationConfigurationDisplayMode(mode.getWidth(), mode.getHeight(), mode.getRefreshRate(),
			mode.getBitDepth());
	}

	public static DisplayMode[] getDisplayModes () {
		GraphicsEnvironment genv = GraphicsEnvironment.getLocalGraphicsEnvironment();
		GraphicsDevice device = genv.getDefaultScreenDevice();
		java.awt.DisplayMode desktopMode = device.getDisplayMode();
		java.awt.DisplayMode[] displayModes = device.getDisplayModes();
		ArrayList<DisplayMode> modes = new ArrayList<DisplayMode>();
		int idx = 0;
		for (java.awt.DisplayMode mode : displayModes) {
			boolean duplicate = false;
			for (int i = 0; i < modes.size(); i++) {
				if (modes.get(i).width == mode.getWidth() && modes.get(i).height == mode.getHeight()
					&& modes.get(i).bitsPerPixel == mode.getBitDepth()) {
					duplicate = true;
					break;
				}
			}
			if (duplicate) continue;
			if (mode.getBitDepth() != desktopMode.getBitDepth()) continue;
			modes.add(new LwjglApplicationConfigurationDisplayMode(mode.getWidth(), mode.getHeight(), mode.getRefreshRate(), mode
				.getBitDepth()));
		}

		return modes.toArray(new DisplayMode[modes.size()]);
	}
}
