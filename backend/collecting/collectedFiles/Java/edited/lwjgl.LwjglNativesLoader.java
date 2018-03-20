

package com.badlogic.gdx.backends.lwjgl;

import static com.badlogic.gdx.utils.SharedLibraryLoader.*;

import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.utils.GdxNativesLoader;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.SharedLibraryLoader;

import java.io.File;
import java.lang.reflect.Method;

public final class LwjglNativesLoader {
	static public boolean load = true;

	static {
		System.setProperty("org.lwjgl.input.Mouse.allowNegativeMouseCoords", "true");

				try {
			Method method = Class.forName("javax.jnlp.ServiceManager").getDeclaredMethod("lookup", new Class[] {String.class});
			method.invoke(null, "javax.jnlp.PersistenceService");
			load = false;
		} catch (Throwable ex) {
			load = true;
		}
	}

	
	static public void load () {
		GdxNativesLoader.load();
		if (GdxNativesLoader.disableNativesLoading) return;
		if (!load) return;

		SharedLibraryLoader loader = new SharedLibraryLoader();
		File nativesDir = null;
		try {
			if (isWindows) {
				nativesDir = loader.extractFile(is64Bit ? "lwjgl64.dll" : "lwjgl.dll", null).getParentFile();
				if (!LwjglApplicationConfiguration.disableAudio)
					loader.extractFileTo(is64Bit ? "OpenAL64.dll" : "OpenAL32.dll", nativesDir);
			} else if (isMac) {
				nativesDir = loader.extractFile("liblwjgl.dylib", null).getParentFile();
				if (!LwjglApplicationConfiguration.disableAudio) loader.extractFileTo("openal.dylib", nativesDir);
			} else if (isLinux) {
				nativesDir = loader.extractFile(is64Bit ? "liblwjgl64.so" : "liblwjgl.so", null).getParentFile();
				if (!LwjglApplicationConfiguration.disableAudio)
					loader.extractFileTo(is64Bit ? "libopenal64.so" : "libopenal.so", nativesDir);
			}
		} catch (Throwable ex) {
			throw new GdxRuntimeException("Unable to extract LWJGL natives.", ex);
		}
		System.setProperty("org.lwjgl.librarypath", nativesDir.getAbsolutePath());
		load = false;
	}
}
