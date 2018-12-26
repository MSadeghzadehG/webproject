

package com.badlogic.gdx.backends.lwjgl3;

import java.io.PrintStream;
import java.nio.IntBuffer;

import org.lwjgl.BufferUtils;
import org.lwjgl.PointerBuffer;
import org.lwjgl.glfw.GLFW;
import org.lwjgl.glfw.GLFWVidMode;
import org.lwjgl.glfw.GLFWVidMode.Buffer;
import org.lwjgl.opengl.GL;

import com.badlogic.gdx.Audio;
import com.badlogic.gdx.Files;
import com.badlogic.gdx.Files.FileType;
import com.badlogic.gdx.Graphics;
import com.badlogic.gdx.Graphics.DisplayMode;
import com.badlogic.gdx.Graphics.Monitor;
import com.badlogic.gdx.Preferences;
import com.badlogic.gdx.audio.Music;
import com.badlogic.gdx.backends.lwjgl3.Lwjgl3Graphics.Lwjgl3Monitor;
import com.badlogic.gdx.graphics.GL20;
import com.badlogic.gdx.graphics.glutils.HdpiUtils;

public class Lwjgl3ApplicationConfiguration extends Lwjgl3WindowConfiguration {
	boolean disableAudio = false;
	int audioDeviceSimultaneousSources = 16;
	int audioDeviceBufferSize = 512;
	int audioDeviceBufferCount = 9;

	boolean useGL30 = false;
	int gles30ContextMajorVersion = 3;
	int gles30ContextMinorVersion = 2;

	int r = 8, g = 8, b = 8, a = 8;
	int depth = 16, stencil = 0;
	int samples = 0;

	boolean vSyncEnabled = true;
	int idleFPS = 60;

	String preferencesDirectory = ".prefs/";
	Files.FileType preferencesFileType = FileType.External;

	HdpiMode hdpiMode = HdpiMode.Logical;

	boolean debug = false;
	PrintStream debugStream = System.err;
	
	static Lwjgl3ApplicationConfiguration copy(Lwjgl3ApplicationConfiguration config) {
		Lwjgl3ApplicationConfiguration copy = new Lwjgl3ApplicationConfiguration();
		copy.set(config);
		return copy;
	}
	
	void set (Lwjgl3ApplicationConfiguration config){
		super.setWindowConfiguration(config);
		disableAudio = config.disableAudio;
		audioDeviceSimultaneousSources = config.audioDeviceSimultaneousSources;
		audioDeviceBufferSize = config.audioDeviceBufferSize;
		audioDeviceBufferCount = config.audioDeviceBufferCount;
		useGL30 = config.useGL30;
		gles30ContextMajorVersion = config.gles30ContextMajorVersion;
		gles30ContextMinorVersion = config.gles30ContextMinorVersion;
		r = config.r;
		g = config.g;
		b = config.b;
		a = config.a;
		depth = config.depth;
		stencil = config.stencil;
		samples = config.samples;
		vSyncEnabled = config.vSyncEnabled;
		preferencesDirectory = config.preferencesDirectory;
		preferencesFileType = config.preferencesFileType;
		hdpiMode = config.hdpiMode;
		debug = config.debug;
		debugStream = config.debugStream;
	}
	
	
	public void setInitialVisible(boolean visibility) {
		this.initialVisible = visibility;
	}

	
	public void disableAudio(boolean disableAudio) {
		this.disableAudio = disableAudio;
	}

	
	public void setAudioConfig(int simultaniousSources, int bufferSize, int bufferCount) {
		this.audioDeviceSimultaneousSources = simultaniousSources;
		this.audioDeviceBufferSize = bufferSize;
		this.audioDeviceBufferCount = bufferCount;
	}

	
	public void useOpenGL3(boolean useGL30, int gles3MajorVersion, int gles3MinorVersion) {
		this.useGL30 = useGL30;
		this.gles30ContextMajorVersion = gles3MajorVersion;
		this.gles30ContextMinorVersion = gles3MinorVersion;
	}

	
	public void setBackBufferConfig(int r, int g, int b, int a, int depth, int stencil, int samples) {
		this.r = r;
		this.g = g;
		this.b = b;
		this.a = a;
		this.depth = depth;
		this.stencil = stencil;
		this.samples = samples;
	}

	
	public void useVsync(boolean vsync) {
		this.vSyncEnabled = vsync;
	}
	
	
	public void setIdleFPS (int fps) {
		this.idleFPS = fps;
	}

	
	public void setPreferencesConfig(String preferencesDirectory, Files.FileType preferencesFileType) {
		this.preferencesDirectory = preferencesDirectory;
		this.preferencesFileType = preferencesFileType;
	}

	
	public void setHdpiMode(HdpiMode mode) {
		this.hdpiMode = mode;
	}

	
	public void enableGLDebugOutput(boolean enable, PrintStream debugOutputStream) {
		debug = enable;
		debugStream = debugOutputStream;
	}

	
	public static DisplayMode getDisplayMode() {
		Lwjgl3Application.initializeGlfw();
		GLFWVidMode videoMode = GLFW.glfwGetVideoMode(GLFW.glfwGetPrimaryMonitor());
		return new Lwjgl3Graphics.Lwjgl3DisplayMode(GLFW.glfwGetPrimaryMonitor(), videoMode.width(), videoMode.height(), videoMode.refreshRate(),
				videoMode.redBits() + videoMode.greenBits() + videoMode.blueBits());
	}
	
	
	public static DisplayMode getDisplayMode(Monitor monitor) {
		Lwjgl3Application.initializeGlfw();
		GLFWVidMode videoMode = GLFW.glfwGetVideoMode(((Lwjgl3Monitor)monitor).monitorHandle);
		return new Lwjgl3Graphics.Lwjgl3DisplayMode(((Lwjgl3Monitor)monitor).monitorHandle, videoMode.width(), videoMode.height(), videoMode.refreshRate(),
				videoMode.redBits() + videoMode.greenBits() + videoMode.blueBits());
	}

	
	public static DisplayMode[] getDisplayModes() {
		Lwjgl3Application.initializeGlfw(); 
		Buffer videoModes = GLFW.glfwGetVideoModes(GLFW.glfwGetPrimaryMonitor());
		DisplayMode[] result = new DisplayMode[videoModes.limit()];
		for (int i = 0; i < result.length; i++) {
			GLFWVidMode videoMode = videoModes.get(i);
			result[i] = new Lwjgl3Graphics.Lwjgl3DisplayMode(GLFW.glfwGetPrimaryMonitor(), videoMode.width(), videoMode.height(),
					videoMode.refreshRate(), videoMode.redBits() + videoMode.greenBits() + videoMode.blueBits());
		}
		return result;
	}

	
	public static DisplayMode[] getDisplayModes(Monitor monitor) {
		Lwjgl3Application.initializeGlfw();
		Buffer videoModes = GLFW.glfwGetVideoModes(((Lwjgl3Monitor)monitor).monitorHandle);
		DisplayMode[] result = new DisplayMode[videoModes.limit()];
		for (int i = 0; i < result.length; i++) {
			GLFWVidMode videoMode = videoModes.get(i);
			result[i] = new Lwjgl3Graphics.Lwjgl3DisplayMode(((Lwjgl3Monitor)monitor).monitorHandle, videoMode.width(), videoMode.height(),
					videoMode.refreshRate(), videoMode.redBits() + videoMode.greenBits() + videoMode.blueBits());
		}
		return result;
	}

	
	public static Monitor getPrimaryMonitor() {
		Lwjgl3Application.initializeGlfw();
		return toLwjgl3Monitor(GLFW.glfwGetPrimaryMonitor());
	}

	
	public static Monitor[] getMonitors() {
		Lwjgl3Application.initializeGlfw();
		PointerBuffer glfwMonitors = GLFW.glfwGetMonitors();
		Monitor[] monitors = new Monitor[glfwMonitors.limit()];
		for (int i = 0; i < glfwMonitors.limit(); i++) {
			monitors[i] = toLwjgl3Monitor(glfwMonitors.get(i));
		}
		return monitors;
	}

	static Lwjgl3Monitor toLwjgl3Monitor(long glfwMonitor) {
		IntBuffer tmp = BufferUtils.createIntBuffer(1);
		IntBuffer tmp2 = BufferUtils.createIntBuffer(1);
		GLFW.glfwGetMonitorPos(glfwMonitor, tmp, tmp2);
		int virtualX = tmp.get(0);
		int virtualY = tmp2.get(0);
		String name = GLFW.glfwGetMonitorName(glfwMonitor);
		return new Lwjgl3Monitor(glfwMonitor, virtualX, virtualY, name);
	}

	public static enum HdpiMode {
		
		Logical,

		
		Pixels
	}
}
